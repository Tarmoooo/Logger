#include <iostream>
#include "LoggerApp.h"

int main() {
    LoggerApp app;

    const char* dllPath =
        "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410PlantEmulator.dll";

    const char* plantsPath = 
        "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410Plants.txt";
        
    int plantNumber = 1;
    app.openLog();
    if (!app.connect(dllPath, plantsPath, plantNumber)) {
        std::cerr << "connect failed\n";
        return 1;
    }

    app.start();

     //std::cout << "Running. Press ENTER to stop...\n";
    std::cin.get();
    

    app.stop();
    app.printData1();
    app.disconnect();

    std::cout << "Exited cleanly\n";
    return 0;
}
