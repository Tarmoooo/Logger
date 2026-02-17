#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <future>
#include <string>
#include <vector>

#include "Data.h"

class EmulatorHost {
public:
    EmulatorHost() = default;
    ~EmulatorHost();

    EmulatorHost(const EmulatorHost&) = delete;
    EmulatorHost& operator=(const EmulatorHost&) = delete;

    bool connect(const char* dllPath, const char* plantsPath, int plantNo);
    void start();
    void stop();
    void pause();
    void resume();
    char state() const;

    void disconnect();

    bool isConnected();
    ControlData& control();


private:
    void setState(char s);

    using SetFn = void(*)(std::string, int);
    using RunFn = void(*)(ControlData*);

    HMODULE h_{nullptr};
    SetFn set_{nullptr};
    RunFn run_{nullptr};

    std::vector<unsigned char> buffer_;
    std::promise<void> finished_;
    ControlData cd_;



};
