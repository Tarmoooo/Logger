#include "EmulatorHost.h"

#include <iostream>

EmulatorHost::~EmulatorHost() {
    // Don’t try to stop threads here (consumer may already be gone).
    // Just unload if still connected.
    disconnect();
}


char EmulatorHost::state() const
{
    return cd_.state.load( );
}

bool EmulatorHost::connect(const char* dllPath, const char* plantsPath, int plantNo) {
    //std::cout << "[connect] Loading DLL...\n";
    h_ = LoadLibraryA(dllPath);
    if (!h_) {
        std::cerr << "[connect] LoadLibraryA failed. GetLastError=" << GetLastError() << "\n";
        return false;
    }

    //std::cout << "[connect] Getting exports...\n";
    set_ = (SetFn)GetProcAddress(h_, "SetIAS0410PlantEmulator");
    run_ = (RunFn)GetProcAddress(h_, "RunIAS0410PlantEmulator");
    if (!set_ || !run_) {
        std::cerr << "[connect] GetProcAddress failed. GetLastError=" << GetLastError() << "\n";
        FreeLibrary(h_);
        h_ = nullptr;
        set_ = nullptr;
        run_ = nullptr;
        return false;
    }

    // Wire ControlData pointers BEFORE calling Set (matches your working code)
    buffer_.clear();
    cd_.pBuf  = &buffer_;
    cd_.pProm = &finished_;
    setState('s'); // start stopped (safe default)

    //std::cout << "[connect] Calling Set...\n";
    try {
        set_(plantsPath, plantNo);
    } catch (...) {
        std::cerr << "[connect] SetIAS0410PlantEmulator threw/aborted\n";
        FreeLibrary(h_);
        h_ = nullptr;
        set_ = nullptr;
        run_ = nullptr;
        return false;
    }

    //std::cout << "[connect] Set finished\n";
    return true;
}

void EmulatorHost::setState(char s) {
    {
        std::lock_guard<std::mutex> lk(cd_.mx);
        cd_.state = s;
    }
    cd_.cv.notify_all();
}



void EmulatorHost::start() {
    setState('r');
    //std::cout << "[start] Calling Run...\n";
    run_(&cd_); // DLL starts detached producer thread
    //std::cout << "[start] Run called\n";
}

void EmulatorHost::stop()   { setState('s'); }
void EmulatorHost::pause()  { setState('b'); }
void EmulatorHost::resume() { setState('r'); }

void EmulatorHost::disconnect() {
    if (h_) {
        FreeLibrary(h_);
        h_ = nullptr;
    }
    set_ = nullptr;
    run_ = nullptr;
}
