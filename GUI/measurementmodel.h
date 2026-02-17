#pragma once
#include "Data.h"
#include <map>
#include <vector>
#include <iostream>


class MeasurementModel
{
public:
    MeasurementModel();
    void addData(const MeasurementData&  data);
    void addHandler(std::function<void(const std::string& input)> handler);
    ChannelMap returnData(const std::string& input);

private:
    ChannelMap data_;
    std::vector<std::function<void(const std::string& input)>> handlers_;

};
