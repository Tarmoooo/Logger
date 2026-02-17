#pragma once

#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <vector>

struct ControlData {
    std::mutex mx;
    std::condition_variable cv;
    std::atomic<char> state{'s'}; // 'r' run, 's' stop, 'b' break
    std::vector<unsigned char>* pBuf{nullptr};
    std::promise<void>* pProm{nullptr};
};
