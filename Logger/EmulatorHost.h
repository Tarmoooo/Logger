#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <future>
#include <string>
#include <vector>

#include "ControlData.h"

class EmulatorHost {
public:
    EmulatorHost() = default;
    ~EmulatorHost(); // best-effort disconnect

    EmulatorHost(const EmulatorHost&) = delete;
    EmulatorHost& operator=(const EmulatorHost&) = delete;

    // Loads DLL, resolves exports, wires ControlData pointers, calls Set(...)
    bool connect(const char* dllPath, const char* plantsPath, int plantNo);

    // State control (assignment commands)
    void start();   // state='r', calls Run(...)
    void stop();    // state='s'
    void pause();   // state='b'
    void resume();  // state='r'
    char state() const;
    // Unload DLL (should only be done when stopped + consumer joined)
    void disconnect();

    bool isConnected() const { return h_ != nullptr; }
    ControlData& control() { return cd_; }

private:
    void setState(char s);

    using SetFn = void(*)(std::string, int); // KEEP: matches your working baseline
    using RunFn = void(*)(ControlData*);

    HMODULE h_{nullptr};
    SetFn set_{nullptr};
    RunFn run_{nullptr};

    std::vector<unsigned char> buffer_;
    std::promise<void> finished_;
    std::future<void> finishedFuture_{finished_.get_future()};

    ControlData cd_{};
};
