#include "packetparser.h"

PacketParser::PacketParser() {}



void PacketParser::parsePacket(const std::vector<unsigned char>& vec, const std::chrono::system_clock::time_point& timep)
{

    MeasurementData data;

    size_t offset= 0;
    data.packetSize=readInt32(vec, offset);

    const int32_t channelCount=readInt32(vec, offset);
    data.timep = timep;

    for (int c = 0; c < channelCount; c++) {
        const int32_t pointCount = readInt32(vec, offset);
        data.channelName   = readString(vec, offset);

        for (int p = 0; p < pointCount; p++) {
            data.pointName = readString(vec, offset);

            if(data.pointName == "Level")
            {
                data.value = readInt32(vec, offset);

            }
            else
            {
                data.value = readDouble(vec, offset);

            }

            for(auto &handler: handlers_)
            {
                handler(data);
            }
        }
    }
}

void PacketParser::addHandler(std::function<void (const MeasurementData &)> handler)
{
    handlers_.push_back(handler);
};

int32_t PacketParser::readInt32(const std::vector<unsigned char>& vec, size_t& offset)
{
    int32_t data;
    memcpy(&data, vec.data() + offset, 4);
    offset+=4;
    return data;
};
double PacketParser::readDouble(const std::vector<unsigned char>& vec, size_t& offset)
{
    double data;
    memcpy(&data, vec.data() + offset, 8);
    offset+=8;
    return data;
};
std::string PacketParser::readString(const std::vector<unsigned char>& vec, size_t& offset)
{

    size_t start = offset;

    while (offset < vec.size() && vec[offset] != 0) offset++;
    if (offset >= vec.size()) throw std::runtime_error("bad string");

    std::string s(reinterpret_cast<const char*>(vec.data() + start), offset - start);

    offset++; // skip '\0'
    return s;
};
