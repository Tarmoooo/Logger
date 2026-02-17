#pragma once

#include <string>
#include "EmulatorHost.h"
#include "PacketConsumer.h"
#include <chrono>
#include <variant>

#include <map>


#include <list>
#include <vector>
#include <cstring>


#include <fstream>
#include <cstdint>
#include <mutex>

#include <ctime>

#include <qobject.h>

class LoggerApp: public QObject {
    Q_OBJECT
public:
    explicit LoggerApp(QObject *parent = nullptr);
    //LoggerApp()=default;
    //LoggerApp():host_(),consumer_(host_.control()) {};

    std::string to_datetime_string(std::chrono::system_clock::time_point tp);
    int32_t read_i32(const std::vector<unsigned char>& b, size_t& off);
    double read_double(const std::vector<unsigned char>& b, size_t& off);
    std::string read_cstr(const std::vector<unsigned char>& b, size_t& off);

    void parse_packet(const std::vector<unsigned char>& b,
                      std::chrono::system_clock::time_point ts);

    void load_log_file(const std::string& path);

    void append_record(const std::vector<unsigned char>& packet,
                       std::chrono::system_clock::time_point ts);
    void openLog();
    void closeLog();

    bool connect(const char* dllPath, const char* plantsPath, int plantNo);
    void disconnect();
    void start();
    void stop();
    void pause();
    void resume();
    void exit();
    void print();
    void printChannelName();
    void printChannelNamePointName();
    void addData1(const std::string& channel,
                  const std::string& point,
                  std::variant<int, double>value,
                  std::chrono::system_clock::time_point ts);
    void printData1();

    static int64_t to_s(std::chrono::system_clock::time_point t);
    
public Q_SLOTS:
    void onConnectRequested(const QString &dllPath,
                            const QString &plantsPath,
                            int plantNo);

Q_SIGNALS:
        void logLine(const QString &line);


private:

    EmulatorHost host_;
    PacketConsumer consumer_{host_.control()};
    std::map<
        std::string,
        std::map<
            std::string,
            std::list<
                std::pair<
                    std::variant<int, double>,
                    std::chrono::system_clock::time_point
                    >
                >
            >
        > Data1;
    static bool isIntPoint(const std::string& channel, const std::string& point);
    std::ofstream logOut_;
    std::mutex logMx_;
};
