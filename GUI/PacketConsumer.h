#pragma once

#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include "ControlData.h"
#include <functional>


class PacketConsumer {
public:
    explicit PacketConsumer(ControlData& cd);
    ~PacketConsumer(); // stop + join (safe cleanup)

    PacketConsumer(const PacketConsumer&) = delete;
    PacketConsumer& operator=(const PacketConsumer&) = delete;

    using Handler = std::function<void(const std::vector<unsigned char>&)>;
    void setHandler(Handler h);


    void start();
    void stop();
    void join();

private:
    void loop();
    
    Handler handler_;
    ControlData& cd_;
    std::atomic<bool> stopFlag_{false};
    std::thread th_;
};
