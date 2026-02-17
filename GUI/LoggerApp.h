#pragma once

#include <QApplication>
#include "loggerui.h"
#include "EmulatorHost.h"
#include "PacketConsumer.h"
#include "packetparser.h"
#include "measurementmodel.h"
#include "filemanager.h"
#include "iostream"
#include "Data.h"


class LoggerApp: public QApplication {
    Q_OBJECT
public:
    explicit LoggerApp(int argc, char *argv[]);


public Q_SLOTS:

Q_SIGNALS:
    void sendMeasurement(const MeasurementData& data);
    void showData(ChannelMap& cmap);

private:
    LoggerUI ui_;
    EmulatorHost host_;
    PacketConsumer consumer_;
    PacketParser parser_;
    MeasurementModel model_;
    FileManager fileMan_;
    std::string textBuffer_;



};

