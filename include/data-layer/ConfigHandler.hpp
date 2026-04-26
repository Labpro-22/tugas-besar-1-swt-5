#pragma once
#include <fstream>
#include <vector>
#include <map>
#include <tuple>
#include <sstream>
#include "./Config.hpp"
#include "./FileIOException.hpp"
using namespace std;

class ConfigHandler
{
protected:
  ifstream configStream;
  string type;

public:
  ConfigHandler(string path, string configType);
  virtual void loadConfig() = 0;
  virtual ~ConfigHandler() = default;
};

class PropertyConfig : public ConfigHandler
{
private:
  vector<tuple<int, string, string, string, string, int, int, vector<int>>> propertyConfigs;
public:
  PropertyConfig(string path);
  void loadConfig();
  vector<tuple<int, string, string, string, string, int, int, vector<int>>> getData();
};

class RailroadConfig: public ConfigHandler
{
private:
  map<int,int> railRoadRentTable;

public:
  RailroadConfig(string path);
  void loadConfig();
  map<int,int> getRailRoadRentTable();
};

class UtilityConfig: public ConfigHandler
{
private:
  map<int,int> utilityMultiplier;

public:
  UtilityConfig(string path);
  void loadConfig();
  map<int,int> getUtilityMultiplier();
};

class TaxConfig : public ConfigHandler
{
private:
  tuple <int,int,int> taxConfig = {0, 0, 0};
public:
  TaxConfig(string path);
  void loadConfig();
  tuple<int,int,int> getTaxConfig();
};

class ActionTileConfig : public ConfigHandler
{
private:
  vector<tuple<int,string,string,string,string>> actionTileConfig;
public:
  ActionTileConfig(string path);
  void loadConfig();
  vector<tuple<int,string,string,string,string>> getActionTileConfig();
};

class SpecialTileConfig : public ConfigHandler
{
private:
  tuple<int,int> specialTileConfig = {0, 0};
public:
  SpecialTileConfig(string path);
  void loadConfig();
  tuple<int,int> getSpecialTileConfig();
};

class MiscTileConfig : public ConfigHandler
{
private:
  tuple<int,int> miscTileConfig = {0, 0};
public:
  MiscTileConfig(string path);
  void loadConfig();
  tuple<int,int> getMiscTileConfig();
};
