
#include "LoggerApp.h"




int64_t LoggerApp::to_s(std::chrono::system_clock::time_point t) {
    return std::chrono::duration_cast<std::chrono::seconds>(
               t.time_since_epoch()).count();
}




void LoggerApp::append_record(const std::vector<unsigned char>& packet,
                              std::chrono::system_clock::time_point ts)
{
    const int64_t s = to_s(ts);

    std::lock_guard<std::mutex> lk(logMx_);

    logOut_.write(reinterpret_cast<const char*>(packet.data()),
              static_cast<std::streamsize>(packet.size()));

    logOut_.write(reinterpret_cast<const char*>(&s),
              static_cast<std::streamsize>(sizeof(s)));

    logOut_.flush(); // optional; you can omit for performance
}

std::string LoggerApp::to_datetime_string(
    std::chrono::system_clock::time_point tp)
{
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);

    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d/%m/%Y %H:%M:%S");
    return oss.str();
}

int32_t LoggerApp::read_i32(const std::vector<unsigned char>& b, size_t& off) {
    if (off + 4 > b.size()) throw std::runtime_error("out of bounds");
    int32_t v;
    std::memcpy(&v, b.data() + off, 4);
    //std::cout<<v<<"\n";
    off += 4;
    return v;
}
double LoggerApp::read_double(const std::vector<unsigned char>& b, size_t& off) {
    if (off + 8 > b.size()) throw std::runtime_error("out of bounds");
    double v;
    std::memcpy(&v, b.data() + off, 8);
    //std::cout<<v<<"\n";
    off += 8;
    return v;
}
std::string LoggerApp::read_cstr(const std::vector<unsigned char>& b, size_t& off) {
    size_t start = off;
    while (off < b.size() && b[off] != 0) off++;
    if (off >= b.size()) throw std::runtime_error("bad string");
    std::string s(reinterpret_cast<const char*>(b.data() + start), off - start);
    //std::cout<<s<<"\n";
    off++; // skip '\0'
    return s;
}
bool LoggerApp::isIntPoint(const std::string& channel, const std::string& point) {
    // Minimal assumption for now:
    return point == "Level";
}



void LoggerApp::load_log_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return; // file doesn't exist yet => fine

    while (true) {
        // 1) read packet length (4 bytes)
        int32_t len = 0;
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
        if (in.eof()) break;          // clean EOF
        if (!in) throw std::runtime_error("Log read failed (length)");

        // basic sanity
        if (len < 8 || len > 1'000'000)
            throw std::runtime_error("Corrupt log (bad packet length)");

        // 2) read the full packet bytes
        std::vector<unsigned char> pkt(static_cast<size_t>(len));
        std::memcpy(pkt.data(), &len, 4); // put the length back into packet[0..3]

        in.read(reinterpret_cast<char*>(pkt.data() + 4),
                static_cast<std::streamsize>(len - 4));
        if (!in) throw std::runtime_error("Corrupt log (truncated packet)");

        // 3) read timestamp seconds (8 bytes)
        int64_t sec = 0;
        in.read(reinterpret_cast<char*>(&sec), sizeof(sec));
        if (!in) throw std::runtime_error("Corrupt log (missing timestamp)");

        // 4) rebuild your data structures using stored timestamp
        auto ts = std::chrono::system_clock::time_point{ std::chrono::seconds(sec) };
        parse_packet(pkt, ts);
    }
}


void LoggerApp::parse_packet(const std::vector<unsigned char>& b,
                             std::chrono::system_clock::time_point ts)
{
    size_t off = 0;

    const int32_t packetLen  = read_i32(b, off);
    const int32_t channelCnt = read_i32(b, off);

    for (int c = 0; c < channelCnt; ++c) {
        const int32_t pointCnt = read_i32(b, off);
        const std::string ch   = read_cstr(b, off);

        for (int p = 0; p < pointCnt; ++p) {
            const std::string pt = read_cstr(b, off);

            if (isIntPoint(ch, pt)) {
                const int32_t v = read_i32(b, off);
                addData1(ch, pt, v, ts);
            } else {
                const double v = read_double(b, off);
                addData1(ch, pt, v, ts);
            }
        }
    }
}





bool LoggerApp::connect(const char* dllPath, 
                        const char* plantsPath, 
                        int plantNo)
{
    return host_.connect(dllPath, plantsPath, plantNo);
};

void LoggerApp::disconnect()
{   
    if(host_.state()!='s')
    {
      return;
    }
    host_.disconnect();
};

//if connected?
void LoggerApp::start()
{
    consumer_.setHandler(
        [this](const std::vector<unsigned char>& packet) {
                        // 1) take timestamp immediately on receipt
            auto ts = std::chrono::system_clock::now();

            // 2) parse packet using that timestamp
            parse_packet(packet, ts);

            // 3) append raw packet + timestamp to log file
            append_record(packet, ts);
            
            
            //parse_packet(packet);
        }
);
    consumer_.start();
    host_.start();
};

//why join?
void LoggerApp::stop()
{
    host_.stop();
    consumer_.stop();
};

void LoggerApp::pause()
{
    host_.pause();
};

void LoggerApp::resume()
{
    host_.resume();
};

void LoggerApp::openLog()
{
    logOut_.open("logfile.bin", std::ios::binary | std::ios::app);
    if (!logOut_) throw std::runtime_error("Failed to open log file");

}

void LoggerApp::addData1(const std::string& channel,
                         const std::string& point,
                         std::variant<int, double> value,
                         std::chrono::system_clock::time_point ts)
{
   //do i need to use insert() first and then update with map[key]=newValue
   Data1[channel][point].push_back({value, ts});
};

void LoggerApp::printData1() {
    //std::lock_guard<std::mutex> lk(dataMx_);

    for (const auto& [channel, points] : Data1) {
        std::cout << "Channel: " << channel << "\n";

        for (const auto& [point, values] : points) {
            std::cout << "  Point: " << point << "\n";

            for (const auto& [val, time] : values) {

                // convert time_point -> seconds since epoch
                //auto sec = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();

                std::cout << "    time =" << to_datetime_string(time)
                          << " value =";

                std::visit([](auto v) {
                    std::cout << v;
                }, val);

                std::cout << "\n";
            }
        }
    }
}


//???return 0?
void LoggerApp::exit()
{

};  

void LoggerApp::print(){};
void LoggerApp::printChannelName(){};
void LoggerApp::printChannelNamePointName(){};

