#include <QApplication>
//#include <iostream>
#include "LoggerApp.h"
#include "loggerui.h"

int main(int argc, char *argv[])
{
    QApplication qapp(argc, argv);

    LoggerApp app;

    const char* dllPath =
        "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410PlantEmulator.dll";

    const char* plantsPath =
        "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410Plants.txt";

    int plantNumber = 1;




    LoggerUI mainWindow;
    mainWindow.show();


    QObject::connect(&mainWindow, &LoggerUI::connectButtonClicked,
                     &mainWindow, [&app,dllPath, plantsPath, plantNumber]() {
                         app.connect(dllPath, plantsPath, plantNumber);
                     });
    QObject::connect(&mainWindow, &LoggerUI::disconnectButtonClicked,
                     &mainWindow, [&app]() {app.disconnect();
                     });

    QObject::connect(&w, &LoggerUI::connectRequested,
                    &app, &LoggerApp::onConnectRequested);

    QObject::connect(&app, &LoggerApp::logLine,
                    &w, &LoggerUI::appendLog);

    return qapp.exec(); // event loop

    /*d
    std::cout<<"Start";
        LoggerApp app;

        const char* dllPath =
            "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410PlantEmulator.dll";

        const char* plantsPath =
            "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410Plants.txt";

        int plantNumber = 1;

        if (!app.connect(dllPath, plantsPath, plantNumber)) {
            std::cerr << "connect failed\n";
            return 1;
        }

        app.start();

        //std::cout << "Running. Press ENTER to stop...\n";
        std::cin.get();


        app.stop();
        //app.printData1();
        app.disconnect();

        std::cout << "Exited cleanly\n";
        return 0;
        */

}
