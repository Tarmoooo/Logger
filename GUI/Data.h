#pragma once

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <variant>
#include <vector>
#include <chrono>
#include <string>
#include <map>

struct ControlData {
    std::mutex mx;
    std::condition_variable cv;
    std::atomic<char> state{'s'}; // 'r' run, 's' stop, 'b' break
    std::vector<unsigned char>* pBuf{nullptr};
    std::promise<void>* pProm{nullptr};
};


struct MeasurementData{
    int32_t packetSize;
    std::chrono::system_clock::time_point timep;
    std::string channelName;
    std::string pointName;
    std::variant<int, double> value;
};

using ChannelMap =
    std::map<
        std::string,
        std::map<
            std::string,
            std::list<
                std::pair<
                    std::variant<int, double>,
                    std::chrono::system_clock::time_point
                        >
                    >
                >
            >;

