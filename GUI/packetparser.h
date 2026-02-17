#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "Data.h"
#include <functional>

class PacketParser
{
public:
    PacketParser();

    void parsePacket(const std::vector<unsigned char>& vec, const std::chrono::system_clock::time_point& timep);
    void addHandler(std::function<void(const MeasurementData& measurementData)> handler);

private:
    int32_t readInt32(const std::vector<unsigned char>& vec, size_t& offset);
    double readDouble(const std::vector<unsigned char>& vec, size_t& offset);
    std::string readString(const std::vector<unsigned char>& vec, size_t& offset);
    std::vector<std::function<void(const MeasurementData& measurementData)>> handlers_;

};
