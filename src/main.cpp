#include "main.h"

#include <iostream>

int main() {
    // 1) OWNER FIRST (creates ControlData here)
    EmulatorHost host;

    // 2) BORROWER AFTER OWNER
    PacketConsumer consumer(host.control());


    // 3) CONNECT + CONFIGURE
    if (!host.connect(
            "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410PlantEmulator.dll",
            "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410Plants.txt",
            1))
    {
        std::cerr << "Failed to connect to emulator DLL\n";
        return 1;
    }
    std::cout << "PacketConsumer created\n";
    // 4) START CONSUMER FIRST
    consumer.start();

    // 5) START DLL PRODUCER
    host.start();

    std::cout << "Running. Press ENTER to stop...\n";
    std::cin.get();

    // =====================================================
    // CLEAN SHUTDOWN (ORDER IS CRITICAL)
    // =====================================================

    // 6) STOP DLL PRODUCER (sets 's' + notify)
    host.stop();

    // 7) WAKE ALL WAITERS (consumer + DLL)
    host.control().cv.notify_all();

    // 8) STOP CONSUMER THREAD
    consumer.requestStop();
    host.control().cv.notify_all();
    consumer.join();

    // 9) UNLOAD DLL ONLY AFTER THREADS ARE DONE
    host.disconnect();

    std::cout << "Exited cleanly\n";
    return 0;
}
