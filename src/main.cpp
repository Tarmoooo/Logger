// main.cpp
// Build (must match the DLL's DEBUG runtime):
//   cl /std:c++17 /EHsc /MDd main.cpp
//
// Run from PowerShell (note the .\ ):
//   .\main.exe

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <atomic>
#include <condition_variable>
#include <future>      // std::promise, std::future
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct ControlData {
    std::mutex mx;
    std::condition_variable cv;
    std::atomic<char> state{'s'}; // 's' stop, 'r' run, 'q' quit
    std::vector<unsigned char>* pBuf{nullptr};
    std::promise<void>* pProm{nullptr};
};

typedef void (*SetFn)(std::string, int);
using RunFn = void(*)(ControlData*);

static void print_last_error(const char* where) {
    DWORD e = GetLastError();
    std::cerr << where << " failed. GetLastError() = " << e << "\n";
}

int main() {
    // Hardcoded for now (simple!)
    const int plantNo = 1;
    const std::string plantsPath = "IAS0410Plants.txt"; // put TXT next to main.exe

    // 1) Load DLL (put DLL next to main.exe)
    HMODULE h = LoadLibraryA("IAS0410PlantEmulator.dll");
    if (!h) {
        print_last_error("LoadLibraryA");
        return 1;
    }
    std::cout << "DLL loaded OK\n";

    // 2) Get exported functions
    SetFn setEmu = (SetFn)GetProcAddress(h, "SetIAS0410PlantEmulator");
    auto runEmu = reinterpret_cast<RunFn>(GetProcAddress(h, "RunIAS0410PlantEmulator"));
    if (!setEmu || !runEmu) {
        print_last_error("GetProcAddress");
        FreeLibrary(h);
        return 1;
    }
    std::cout << "Exports found OK\n";

    // 3) Shared control + buffer
    std::vector<unsigned char> buffer;
    std::promise<void> finished;
    std::future<void> finishedFuture = finished.get_future();

    ControlData cd;
    cd.pBuf = &buffer;
    cd.pProm = &finished;
    cd.state = 'r'; // start stopped

    // 4) Configure emulator
    std::cout << "Calling Set...\n";
    try {
        setEmu(plantsPath, plantNo);
    } catch (...) {
        std::cerr << "SetIAS0410PlantEmulator threw an exception.\n";
        FreeLibrary(h);
        return 1;
    }
    std::cout << "Set finished\n";

    // 5) Consumer thread: minimal handshake (prints buffer size)
    std::atomic<bool> stopConsumer{false};
    std::thread consumer([&] {
        while (!stopConsumer.load()) {
            std::unique_lock<std::mutex> lk(cd.mx);
            
            cd.cv.wait(lk); // DLL notifies when it fills buffer / changes state

            char st = cd.state.load();
            if (st == 'q') break;
            if (st != 'r') continue;

            // If DLL uses empty-notifies, ignore empties
            if (!cd.pBuf || cd.pBuf->empty()) {
                continue;
            }

            std::cout << "Got data! Buffer size = " << cd.pBuf->size() << " bytes\n";

            // Mark consumed so DLL can produce next packet
            cd.pBuf->clear();

            lk.unlock();
            cd.cv.notify_one();
        }
    });

    // 6) Run emulator in a separate thread
    std::cout << "Calling Run...\n";
    try {
        runEmu(&cd);              // starts detached producer inside DLL
        std::cout << "Run called\n";
    } catch (const std::exception& e) {
        std::cerr << "Run threw std::exception: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Run threw an unknown exception\n";
    }



    std::cout << "Running. Press ENTER to quit...\n";
    std::cin.get();

    // 8) Quit cleanly (prevent abort/unload while threads still running)
    {
        std::lock_guard<std::mutex> lk(cd.mx);
        cd.state = 's';
    }
    cd.cv.notify_all();

    // Wait for DLL to signal it finished (promise)
    // If this hangs, the DLL may use a different quit mechanism; tell me and we’ll adjust.
    finishedFuture.wait();

    stopConsumer = true;
    cd.cv.notify_all();

    if (consumer.joinable()) consumer.join();

    FreeLibrary(h);
    std::cout << "Exited cleanly\n";
    return 0;
}
