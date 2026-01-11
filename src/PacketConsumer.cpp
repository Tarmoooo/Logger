#include "PacketConsumer.h"

#include <iostream>

PacketConsumer::PacketConsumer(ControlData& cd)
    : cd_(cd) {}

PacketConsumer::~PacketConsumer() {
    requestStop();
    join();
}

void PacketConsumer::start() {
    stopFlag_ = false;
    th_ = std::thread([this] { loop(); });
}

void PacketConsumer::requestStop() {
    stopFlag_ = true;
    cd_.cv.notify_all();
}

void PacketConsumer::join() {
    if (th_.joinable()) th_.join();
}

void PacketConsumer::loop() {
    while (!stopFlag_) {
        std::vector<unsigned char> local;

        {
            std::unique_lock<std::mutex> lk(cd_.mx);

            // Predicate prevents missed/spurious wakeups
            cd_.cv.wait(lk, [&] {
                return stopFlag_
                    || cd_.state.load() != 'r'
                    || (cd_.pBuf && !cd_.pBuf->empty());
            });

            if (stopFlag_) break;

            const char st = cd_.state.load();
            if (st == 's') break;     // stop => terminate consumer
            if (st == 'b') continue;  // break => paused

            if (!cd_.pBuf || cd_.pBuf->empty()) continue;

            local = *cd_.pBuf;        // copy out quickly
            cd_.pBuf->clear();        // mark consumed
        }

        cd_.cv.notify_one();          // ACK producer

        // TODO: parse(local), store, log with timestamp
        std::cout << "Got data! Buffer size = " << local.size() << " bytes\n";
    }
}
