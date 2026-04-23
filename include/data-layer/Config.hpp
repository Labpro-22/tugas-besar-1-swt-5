#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <vector>
#include <map>
#include <string>
#include <tuple>
using namespace std;

// Jenis Tax
const int PPH = 0;
const int PERSEN = 1;
const int PBM = 2;

// Jenis Petak Spesial
const int GO_SALARY = 0;
const int JAIL_FINE = 1;

//Jenis Misc Config
const int MAX_TURN = 0;
const int SALDO_AWAL = 1;

class Config
{
private:
    vector<tuple<int, string, string, string, string, int, int, vector<int>>> propertyConfigs;
    map<int, int> railroadRentTable;
    map<int, int> utilityMultiplier;
    tuple<int, int, int> taxConfig;
    tuple<int, int> specialConfig;
    tuple<int, int> miscConfig;
    tuple<int, string, string, string, string> actionTileConfig;

public:
    Config(
        vector<tuple<int, string, string, string, string, int, int, vector<int>>> propertyConfigs,
        map<int, int> railroadRentTable,
        map<int, int> utilityMultiplier,
        tuple<int, int, int> taxConfig,
        tuple<int, int> specialConfig,
        tuple<int, int> miscConfig,
        tuple<int, string, string, string, string> actionTileConfig
    );
    vector<tuple<int, string, string, string, string, int, int, vector<int>>> getPropertyConfig();
    int getRailroadRent(int count);
    int getUtilityMultiplier(int count);
    int getTaxConfig(int taxTypeIdx); //PPH, PERSEN, PBM
    int getSpecialConfig(int specialIdx); // GO_SALARY, JAIL_FINE
    int getMiscConfig(int miscIdx); // MAX_TURN, SALDO_AWAL
    tuple<int, string, string, string, string> getActionTileConfig();
};

#endif