#include "../../include/data-layer/Config.hpp"

tuple<int, string, string, string, string, int, int, vector<int>> Config::getPropertyConfig(string code)
{
    int idx = 0;
    for (size_t i = 0; i < this->propertyConfigs.size();i++) {
        if (get<1>(this->propertyConfigs[idx]).compare(code) == 0) {
            break;
        }
        idx++;
    }
    return this->propertyConfigs[idx];
} 

int Config::getRailroadRent(int count)
{
    auto iter = railroadRentTable.find(count);
    if (iter != railroadRentTable.end()){
        return iter->second;
    }
    return -1;
}

int Config::getUtilityMultiplier(int count)
{
    auto iter = utilityMultiplier.find(count);
    if (iter != utilityMultiplier.end()){
        return iter->second;
    }
    return -1;
}