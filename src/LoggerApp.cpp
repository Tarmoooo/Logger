
#include "LoggerApp.h"



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


void LoggerApp::parse_packet(const std::vector<unsigned char>& b) {
    //std::cout << "parse_packet: buffer size = " << b.size() << "\n";
    size_t off = 0;

    // 1) packet length
    const int32_t packetLen = read_i32(b, off);

    // 2) number of channels
    const int32_t channelCnt = read_i32(b, off);

    // timestamp for all values in this packet
    const auto ts = std::chrono::system_clock::now();

    // 3) channels
    for (int c = 0; c < channelCnt; ++c) {
        // channel: point count + channel name
        const int32_t pointCnt = read_i32(b, off);
        const std::string ch   = read_cstr(b, off);

        // points
        for (int p = 0; p < pointCnt; ++p) {
            const std::string pt = read_cstr(b, off);

            // value: int32 or double
            if (isIntPoint(ch, pt)) {
                const int32_t v = read_i32(b, off);
                addData1(ch, pt, v, ts);
                // std::cout << ch << " / " << pt << " = " << v << " (int)\n";
            } else {
                const double v = read_double(b, off);
                addData1(ch, pt, v, ts);
                // std::cout << ch << " / " << pt << " = " << v << " (double)\n";
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
            parse_packet(packet);
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


void LoggerApp::addData1(const std::string& channel,
                         const std::string& point,
                         std::variant<int, double> value,
                         std::chrono::system_clock::time_point ts)
{
   //do i need to use insert() first and then update with map[key]=newValue
   Data1[channel][point].push_back({value, ts});
};

void LoggerApp::printData1() {
    for (const auto& [channel, points] : Data1) {
        std::cout << "Channel: " << channel << "\n";

        for (const auto& [point, values] : points) {
            std::cout << "  Point: " << point << "\n";

            for (const auto& [val, time] : values) {
                std::visit([](auto v) {
                    std::cout << "    value = " << v << "\n";
                }, val);
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

