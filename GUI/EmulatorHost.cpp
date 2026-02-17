#include "EmulatorHost.h"

#include <iostream>

static const char* dllPath =
    "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410PlantEmulator.dll";
static const char* plantsPath =
    "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410Plants.txt";
static int plantNumber = 1;

EmulatorHost::~EmulatorHost() {
}

char EmulatorHost::state() const
{
    return cd_.state.load( );
}

bool EmulatorHost::connect(const char* dllPath, const char* plantsPath, int plantNo) {
    if(h_) return 1;

    h_ = LoadLibraryA(dllPath);
    if (!h_) {
        std::cerr << "LoadLibraryA failed. GetLastError=" << GetLastError() << "\n";
        return false;
    }

    set_ = (SetFn)GetProcAddress(h_, "SetIAS0410PlantEmulator");
    run_ = (RunFn)GetProcAddress(h_, "RunIAS0410PlantEmulator");
    if (!set_ || !run_) {
        std::cerr << "GetProcAddress failed. GetLastError=" << GetLastError() << "\n";
        FreeLibrary(h_);
        h_ = nullptr;
        set_ = nullptr;
        run_ = nullptr;
        return false;
    }

    buffer_.clear();
    cd_.pBuf  = &buffer_;
    cd_.pProm = &finished_;
    setState('s');

    try {
        set_(plantsPath, plantNo);
    } catch (...) {
        std::cerr << " SetIAS0410PlantEmulator threw/aborted\n";
        FreeLibrary(h_);
        h_ = nullptr;
        set_ = nullptr;
        run_ = nullptr;
        return false;
    }
    return true;
}

void EmulatorHost::setState(char s) {

    cd_.state = s;
    cd_.cv.notify_all();
}

ControlData& EmulatorHost::control() { return cd_;}

void EmulatorHost::start() {

    if (state() == 'r' || state() == 'b' || isConnected()!=1) return;

    buffer_.clear();
    finished_ = std::promise<void>();

    setState('r');

    if (run_) {
        run_(&cd_);
        std::cout << "Runnning\n";
    }

}

void EmulatorHost::stop()   { setState('s'); }

void EmulatorHost::pause(){

    if(state()=='s') return;
    setState('b');
    std::cout<<"Pausing\n";
}
void EmulatorHost::resume(){

    if(state()=='b') setState('r');
    std::cout<<"Resuming\n";
}

void EmulatorHost::disconnect() {
    if(state()!='s') return;
    if (h_) {
        FreeLibrary(h_);
        h_ = nullptr;
    }
    set_ = nullptr;
    run_ = nullptr;
    std::cout<<"Disconnecting\n";
}

bool EmulatorHost::isConnected(){
    return h_ != nullptr;
}
