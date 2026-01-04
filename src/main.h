#pragma once

// ===== Windows / DLL =====
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// ===== STL =====
#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>

// ======================================================
// Shared ControlData (exactly as DLL expects)
// ======================================================
struct ControlData {
    std::mutex mx;
    std::condition_variable cv;
    std::atomic<char> state{'s'};   // 'r' run, 's' stop, 'b' break
    std::vector<unsigned char>* pBuf{nullptr};
    std::promise<void>* pProm{nullptr};
};

// ======================================================
// PacketConsumer
// ======================================================
class PacketConsumer {
public:
    explicit PacketConsumer(ControlData& cd)
        : cd_(cd) {}

    void start() {
        stopFlag_ = false;
        th_ = std::thread([this]{ loop(); });
    }

    void requestStop() {
        stopFlag_ = true;
        cd_.cv.notify_all();
    }

    void join() {
        if (th_.joinable()) th_.join();
    }

private:
    void loop() {
        while (!stopFlag_) {
            std::vector<unsigned char> local;

            {
                std::unique_lock<std::mutex> lk(cd_.mx);
                cd_.cv.wait(lk, [&]{
                    return stopFlag_
                        || cd_.state.load() != 'r'
                        || (cd_.pBuf && !cd_.pBuf->empty());
                });

                if (stopFlag_) break;
                if (cd_.state.load() == 's') break;
                if (cd_.state.load() == 'b') continue;

                if (!cd_.pBuf || cd_.pBuf->empty()) continue;

                local = *cd_.pBuf;
                cd_.pBuf->clear();
            }

            cd_.cv.notify_one();   // ACK producer

            // TODO: parse(local), store, log with timestamp
            std::cout << "Packet received: " << local.size() << " bytes\n";
        }
    }

    ControlData& cd_;
    std::atomic<bool> stopFlag_{false};
    std::thread th_;
};

// ======================================================
// EmulatorHost
// ======================================================
class EmulatorHost {
public:
    bool connect(const char* dllPath,
                 const std::string& plantsPath,
                 int plantNo)
    {
        std::cout << "[connect] Loading DLL...\n";
        h_ = LoadLibraryA(dllPath);
        if (!h_) {
            std::cerr << "[connect] LoadLibraryA failed. GetLastError=" << GetLastError() << "\n";
            return false;
        }
        std::cout << "[connect] DLL loaded\n";

        std::cout << "[connect] Getting exports...\n";
        set_ = (SetFn)GetProcAddress(h_, "SetIAS0410PlantEmulator");
        run_ = (RunFn)GetProcAddress(h_, "RunIAS0410PlantEmulator");
        if (!set_ || !run_) {
            std::cerr << "[connect] GetProcAddress failed. GetLastError=" << GetLastError() << "\n";
            FreeLibrary(h_);
            h_ = nullptr;
            return false;
        }
        std::cout << "[connect] Exports found\n";

        // IMPORTANT: set ControlData pointers exactly like your working version
        buffer_.clear();
        cd_.pBuf  = &buffer_;
        cd_.pProm = &finished_;
        cd_.state = 's'; // start stopped (safe default)

        std::cout << "[connect] Calling Set...\n";
        try {
            set_(plantsPath, plantNo); // std::string ABI (same as your working code)
        } catch (...) {
            std::cerr << "[connect] SetIAS0410PlantEmulator threw/aborted\n";
            FreeLibrary(h_);
            h_ = nullptr;
            return false;
        }
        std::cout << "[connect] Set finished\n";

        return true;
    }

    void start() {
        // start producing
        {
            std::lock_guard<std::mutex> lk(cd_.mx);
            cd_.state = 'r';
        }
        cd_.cv.notify_all();

        std::cout << "[start] Calling Run...\n";
        run_(&cd_); // starts detached producer inside DLL (as in your working code)
        std::cout << "[start] Run called\n";
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lk(cd_.mx);
            cd_.state = 's';
        }
        cd_.cv.notify_all();
    }

    void disconnect() {
        if (h_) {
            FreeLibrary(h_);
            h_ = nullptr;
        }
    }

    ControlData& control() { return cd_; }

private:
    using SetFn = void(*)(std::string, int);
    using RunFn = void(*)(ControlData*);

    HMODULE h_{nullptr};
    SetFn set_{nullptr};
    RunFn run_{nullptr};

    std::vector<unsigned char> buffer_;
    std::promise<void> finished_;
    ControlData cd_{};
};

