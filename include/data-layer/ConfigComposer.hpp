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
public:
    Config getConfig();
    void buildConfig(string path);
};