
#include "LoggerApp.h"
static const char* dllPath =
    "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410PlantEmulator.dll";
static const char* plantsPath =
    "C:/Users/Admin/Documents/PROJECT C++ Programmeerimine/src/IAS0410Plants.txt";
static int plantNumber = 1;

#include <iostream>
LoggerApp::LoggerApp(int argc, char *argv[])
    :QApplication(argc, argv), consumer_(host_.control())
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    connect(&ui_, &LoggerUI::connectButtonClicked,
            this, [this](){
                host_.connect(dllPath, plantsPath, plantNumber);
            });

    connect(&ui_, &LoggerUI::disconnectButtonClicked,
            this, [this](){
        if(host_.state()!='s') return;
                consumer_.stop();
                consumer_.join();
                host_.disconnect();
    });

    //TODO: check if connect=1
    connect(&ui_, &LoggerUI::startButtonClicked,
            this, [this](){
        if(host_.isConnected()!=1) return;
        /*
                consumer_.stop();
                consumer_.join();
        */
                consumer_.start();
                host_.start();
            });

    connect(&ui_, &LoggerUI::stopButtonClicked,
            this, [this](){

            consumer_.stop();
                host_.stop();
                //consumer_.stop();
            });

    connect(&ui_, &LoggerUI::pauseButtonClicked,
            this, [this](){
            host_.pause();
                });

    connect(&ui_, &LoggerUI::resumeButtonClicked,
            this, [this](){

                host_.resume();

            });

    connect(&ui_, &LoggerUI::openButtonClicked,
            this, [this](){
                // 1. Show the dialog
                QString fileName = QFileDialog::getOpenFileName(&ui_, "Open Saved Log", "", "Binary Files (*.bin)");
                std::string fName= fileName.toStdString();
                fileMan_.openFile(fName);
            });

    connect(this, &LoggerApp::sendMeasurement, &ui_, &LoggerUI::onSendMeasurement);

    connect(this, &LoggerApp::showData, &ui_, &LoggerUI::onShowData);


    connect(&ui_, &LoggerUI::showDataButtonClicked, this, [this](const QString& channelName) {
        if(host_.isConnected()) return;
        ChannelMap result = model_.returnData(channelName.toStdString());
        Q_EMIT showData(result);

    });


    consumer_.addHandler([this](const std::vector<unsigned char>& packet, const std::chrono::system_clock::time_point& timep)
                         {
                             parser_.parsePacket(packet, timep);

                         });

    parser_.addHandler([this](const MeasurementData& data)
                       {
                           model_.addData(data);
                           Q_EMIT sendMeasurement(data);

                       });

    consumer_.addHandler([this](const std::vector<unsigned char>& vec, const std::chrono::system_clock::time_point& timep)
                         {
                             fileMan_.saveToFile(vec, timep);
                         });
    fileMan_.addHandler([this](const std::vector<unsigned char>& vec, const std::chrono::system_clock::time_point& timep)
                        {
                            parser_.parsePacket(vec, timep);
                        });



    ui_.show();

}


