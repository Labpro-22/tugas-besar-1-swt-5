#include "../../include/data-layer/Config.hpp"

Config::Config(
        vector<tuple<int, string, string, string, string, int, int, vector<int>>> propertyConfigs,
        map<int, int> railroadRentTable,
        map<int, int> utilityMultiplier,
        tuple<int, int, int> taxConfig,
        tuple<int, int> specialConfig,
        tuple<int, int> miscConfig,
        tuple<int, string, string, string, string> actionTileConfig
)
{
    this->propertyConfigs = propertyConfigs;
    this->railroadRentTable = railroadRentTable;
    this->utilityMultiplier = utilityMultiplier;
    this->taxConfig = taxConfig;
    this->specialConfig = specialConfig;
    this->miscConfig = miscConfig;
    this->actionTileConfig = actionTileConfig;
};

vector<tuple<int, string, string, string, string, int, int, vector<int>>> Config::getPropertyConfig()
{
    return this->propertyConfigs;
}

int Config::getRailroadRent(int count)
{
    int maxQt = -99;
    int minQt = 99;
    int maxQtVal = 0;
    int minQtVal = 0;
    for (auto i : this->railroadRentTable){
        if (i.first > maxQt) {
           maxQt = i.first; 
           maxQtVal = i.second;
        }
        if (i.first < minQt) {
            minQt = i.first;
            minQtVal = i.second;
        }
    }

    if (count >= minQt && count <= maxQt) {
        auto iter = railroadRentTable.find(count);
        if (iter != railroadRentTable.end()){
            return iter->second;
        }
    }
    
    if (count < minQt) return minQtVal;
    return maxQtVal;
}

int Config::getUtilityMultiplier(int count)
{
    int maxQt = -99;
    int minQt = 99;
    int maxQtVal = 0;
    int minQtVal = 0;
    for (auto i : this->utilityMultiplier){
        if (i.first > maxQt) {
           maxQt = i.first; 
           maxQtVal = i.second;
        }
        if (i.first < minQt) {
            minQt = i.first;
            minQtVal = i.second;
        }
    }

    if (count >= minQt && count <= maxQt) {
        auto iter = railroadRentTable.find(count);
        if (iter != railroadRentTable.end()){
            return iter->second;
        }
    }
    
    if (count < minQt) return minQtVal;
    return maxQtVal;
}

    
int Config::getTaxConfig(int taxTypeIdx) //PPH, PERSEN, PBM
{
    switch (taxTypeIdx) {
        case(PPH): return get<PPH>(this->taxConfig);
        case(PERSEN): return get<PERSEN>(this->taxConfig);
        case(PBM): return get<PBM>(this->taxConfig);
        default: return 0;
    }
}

int Config::getSpecialConfig(int specialIdx) // GO_SALARY, JAIL_FINE
{
    switch (specialIdx) {
        case(GO_SALARY): return get<GO_SALARY>(this->specialConfig);
        case(JAIL_FINE): return get<JAIL_FINE>(this->specialConfig);
        default: return 0;
    }
}
int Config::getMiscConfig(int miscIdx) // MAX_TURN, SALDO_AWAL
{
    switch(miscIdx) {
        case(MAX_TURN): return get<MAX_TURN>(this->miscConfig);
        case(SALDO_AWAL): return get<JAIL_FINE>(this->miscConfig);
        default: return 0;
    }
}
tuple<int, string, string, string, string> Config::getActionTileConfig()
{
    return this->actionTileConfig;
}