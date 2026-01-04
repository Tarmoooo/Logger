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
    bool connect(const char* dllName,
                 const char* plantsPath,
                 int plantNo)
    {
        h_ = LoadLibraryA(dllName);
        if (!h_) {
            std::cerr << "LoadLibrary failed\n";
            return false;
        }

        set_ = reinterpret_cast<SetFn>(
            GetProcAddress(h_, "SetIAS0410PlantEmulator"));
        run_ = reinterpret_cast<RunFn>(
            GetProcAddress(h_, "RunIAS0410PlantEmulator"));

        if (!set_ || !run_) {
            std::cerr << "GetProcAddress failed\n";
            FreeLibrary(h_);
            h_ = nullptr;
            return false;
        }

        buffer_.clear();
        cd_.pBuf = &buffer_;
        cd_.pProm = &finished_;

        set_(plantsPath, plantNo);
        return true;
    }

    void start() {
        setState('r');
        run_(&cd_);   // starts detached producer
    }

    void pause()  { setState('b'); }
    void resume() { setState('r'); }
    void stop()   { setState('s'); }

    void disconnect() {
        if (h_) {
            FreeLibrary(h_);
            h_ = nullptr;
        }
    }

    ControlData& control() { return cd_; }

private:
    using SetFn = void(*)(const char*, int);
    using RunFn = void(*)(ControlData*);

    void setState(char s) {
        {
            std::lock_guard<std::mutex> lk(cd_.mx);
            cd_.state = s;
        }
        cd_.cv.notify_all();
    }

    HMODULE h_{nullptr};
    SetFn set_{nullptr};
    RunFn run_{nullptr};

    std::vector<unsigned char> buffer_;
    std::promise<void> finished_;
    ControlData cd_{};
};
