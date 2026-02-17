#include "PacketConsumer.h"



PacketConsumer::PacketConsumer(ControlData& cd)
    : cd_(cd) {}

PacketConsumer::~PacketConsumer() {
    stop();
    join();
}

void PacketConsumer::addHandler(std::function<void(const std::vector<unsigned char>&, const std::chrono::system_clock::time_point&)> handler)
{
    handlers_.push_back(handler);
}

void PacketConsumer::start() {
    if (th_.joinable()) return;
    stopFlag_ = false;
    th_ = std::thread([this] { loop(); });
}


void PacketConsumer::stop() {
    stopFlag_ = true;
    cd_.cv.notify_all();
    //cd_.state.store('s');
    join();

}

void PacketConsumer::join() {
    if (th_.joinable()) th_.join();
}

void PacketConsumer::loop() {
    while (!stopFlag_) {
        std::vector<unsigned char> local;
        {
            std::unique_lock<std::mutex> lk(cd_.mx);

            cd_.cv.wait(lk, [&] {
                return stopFlag_ || !cd_.pBuf->empty() || cd_.state.load() == 's';

            });

            if (cd_.pBuf && !cd_.pBuf->empty()) {
                local = *cd_.pBuf;
                cd_.pBuf->clear();
            }
                /*
            } else {
                continue;
            }
                */
            if (stopFlag_ || cd_.state.load() == 's') break;

        }

        cd_.cv.notify_one();

        std::chrono::system_clock::time_point timep = std::chrono::system_clock::now();
        for(auto &handler: handlers_)
        {
            handler(local, timep);
        }

        std::cout << "Got data! Buffer size = " << local.size() << " bytes\n";
    }


}
