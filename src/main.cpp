#include "main.h"

int main() {
    EmulatorHost host;

    if (!host.connect("IAS0410PlantEmulator.dll",
                      "IAS0410Plants.txt",
                      1)) {
        return 1;
    }

    PacketConsumer consumer(host.control());

    consumer.start();
    host.start();

    std::cout << "Press ENTER to stop...\n";
    std::cin.get();

    host.stop();
    consumer.requestStop();
    consumer.join();
    host.disconnect();

    std::cout << "Exited cleanly\n";
}
