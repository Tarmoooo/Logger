#pragma once

#include <atomic>
#include <thread>
#include <vector>

#include "ControlData.h"

class PacketConsumer {
public:
    explicit PacketConsumer(ControlData& cd);
    ~PacketConsumer(); // stop + join (safe cleanup)

    //PacketConsumer(const PacketConsumer&) = delete;
    //PacketConsumer& operator=(const PacketConsumer&) = delete;

    void start();
    void requestStop();
    void join();

private:
    void loop();

    ControlData& cd_;
    std::atomic<bool> stopFlag_{false};
    std::thread th_;
};
