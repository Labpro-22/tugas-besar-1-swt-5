#pragma once
#include <vector>
#include <map>
#include <tuple>
#include "./ConfigHandler.hpp"
#include "./Config.hpp"
using namespace std;

class ConfigComposer
{
private:
    Config currentConfig;
    static Config buildConfig(const string &pathProp, const string &pathRail, const string &pathUtil, const string &pathTax, const string &pathAction, const string &pathSpecial, const string &pathMisc);

public:
    ConfigComposer(const string &pathProp, const string &pathRail, const string &pathUtil, const string &pathTax, const string &pathAction, const string &pathSpecial, const string &pathMisc);
    Config& getConfig();
};