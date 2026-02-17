#include "measurementmodel.h"


MeasurementModel::MeasurementModel() {}

void MeasurementModel::addData(const MeasurementData  &data)
{

    data_[data.channelName][data.pointName].push_back({data.value, data.timep});
}


void MeasurementModel::addHandler(std::function<void(const std::string& input)> handler)
{
    handlers_.push_back(handler);
}



ChannelMap MeasurementModel::returnData(const std::string& input)
{
    ChannelMap result;
    auto it = data_.find(input);
    if (it == data_.end()) return result;
    result.insert(*it);
    return result;

}

