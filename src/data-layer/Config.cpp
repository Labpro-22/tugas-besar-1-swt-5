#include "../../include/data-layer/Config.hpp"

<<<<<<< Updated upstream
tuple<int, string, string, string, string, int, int, vector<int>> Config::getPropertyConfig(string code)
=======
Config::Config(
        vector<tuple<int, string, string, string, string, int, int, vector<int>>> propertyConfigs,
        map<int, int> railroadRentTable,
        map<int, int> utilityMultiplier,
        tuple<int, int, int> taxConfig,
        tuple<int, int> specialConfig,
        tuple<int, int> miscConfig,
        vector<tuple<int, string, string, string, string>> actionTileConfig
)
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
    return -1;
=======

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
vector<tuple<int, string, string, string, string>> Config::getActionTileConfig()
{
    return this->actionTileConfig;
>>>>>>> Stashed changes
}