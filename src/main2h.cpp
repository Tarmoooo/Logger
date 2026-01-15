#include <iostream>

#include "EmulatorHost.h"
#include "PacketConsumer.h"

int main() {
    EmulatorHost host;                      // owner first
    PacketConsumer consumer(host.control()); // borrower after

    const char* dllPath =
        "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410PlantEmulator.dll";

    if (!host.connect(dllPath, "IAS0410Plants.txt", 1)) {
        std::cerr << "connect failed\n";
        return 1;
    }

    consumer.start();
    host.start();

    std::cout << "Running. Press ENTER to stop...\n";
    std::cin.get();

    host.stop();                 // tell DLL to stop
    host.control().cv.notify_all();

    consumer.requestStop();      // stop consumer thread
    host.control().cv.notify_all();
    consumer.join();

    host.disconnect();           // unload DLL last
    std::cout << "Exited cleanly\n";
    return 0;
}
