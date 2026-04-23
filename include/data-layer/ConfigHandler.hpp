#pragma once
#include <fstream>
#include <vector>
#include <map>
#include <tuple>
#include "./Config.hpp"
using namespace std;

class ConfigHandler
{
protected:
  ifstream configStream;
  string type;

public:
  ConfigHandler(string path, string configType);
  virtual void loadConfig();
  ifstream getStream();
};

class PropertyConfig : public ConfigHandler
{
public:
  PropertyConfig(string path);
  void loadConfig();
};

class PublicFacilityConfig : public ConfigHandler
{
public:
  PublicFacilityConfig(string path);
  void loadConfig();
};

class TaxConfig : public ConfigHandler
{
public:
  TaxConfig(string path);
  void loadConfig();
};

class ActionTileConfig : public ConfigHandler
{
public:
  ActionTileConfig(string path);
  void loadConfig();
};

class SpecialTileConfig : public ConfigHandler
{
public:
  SpecialTileConfig(string path);
  void loadConfig();
};

class MiscTileConfig : public ConfigHandler
{
public:
  MiscTileConfig(string path);
  void loadConfig();
};

class ConfigCompose
{
private:
  Config currentConfig;
public:
  Config getConfig();
  void buildConfig(string path);
};