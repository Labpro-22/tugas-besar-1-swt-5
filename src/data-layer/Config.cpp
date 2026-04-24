#include "../../include/data-layer/Config.hpp"

Config::Config(
        vector<tuple<int, string, string, string, string, int, int, vector<int>>> propertyConfigs,
        map<int, int> railroadRentTable,
        map<int, int> utilityMultiplier,
        tuple<int, int, int> taxConfig,
        tuple<int, int> specialConfig,
        tuple<int, int> miscConfig,
        vector<tuple<int, string, string, string, string>> actionTileConfig
) : 
    propertyConfigs(propertyConfigs),
    railroadRentTable(railroadRentTable),
    utilityMultiplier(utilityMultiplier),
    taxConfig(taxConfig),
    specialConfig(specialConfig),
    miscConfig(miscConfig),
    actionTileConfig(actionTileConfig){}

tuple<int, string, string, string, string, int, int, vector<int>> Config::getPropertyConfig(string code)
{
    for (auto i = this->propertyConfigs.begin(); i != this->propertyConfigs.end(); i++)
    {
        if (get<1>(*i).compare(code) == 0){
            return *i;
        }
    }
    auto notFound = make_tuple(-1, "nan", "nan", "nan", "nan", -1, -1, vector<int>{-1, -2, -3});
    return notFound;
}

vector<tuple<int, string, string, string, string, int, int, vector<int>>> Config::getPropertyConfigAll()
{
    return this->propertyConfigs;
}
int Config::getRailroadRent(int count)
{
    int minQt = 99;
    int maxQt = -99;
    int maxQtVal = 0;
    int minQtVal = 0;

    auto iter = railroadRentTable.find(count);
    if (iter != railroadRentTable.end()){
        return iter->second;
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
    int minQt = 99;
    int maxQt = -99;
    int maxQtVal = 0;
    int minQtVal = 0;

    auto iter = utilityMultiplier.find(count);
    if (iter != utilityMultiplier.end()){
        return iter->second;
    }

    if (count >= minQt && count <= maxQt) {
        auto iter = utilityMultiplier.find(count);
        if (iter != utilityMultiplier.end()){
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
        case(SALDO_AWAL): return get<SALDO_AWAL>(this->miscConfig);
        default: return 0;
    }
}
vector<tuple<int, string, string, string, string>> Config::getActionTileConfigAll()
{
    return this->actionTileConfig;
}

tuple<int,string,string,string,string> Config::getActionTileConfig(string code)
{
     for (auto i = this->actionTileConfig.begin(); i != this->actionTileConfig.end(); i++)
    {
        if (get<1>(*i).compare(code) == 0){
            return *i;
        }
    }
    auto notFound = make_tuple(-1, "nan", "nan", "nan", "nan");
    return notFound;
}