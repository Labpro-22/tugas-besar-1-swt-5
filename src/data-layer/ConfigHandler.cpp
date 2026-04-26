#include "../../include/data-layer/ConfigHandler.hpp"
#include <stdexcept>

namespace {
std::vector<std::string> splitTokens(const std::string& line) {
  std::istringstream iss(line);
  std::vector<std::string> tokens;
  std::string token;
  while (iss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

int parseIntToken(const std::string& token, const std::string& type, int lineNo, const std::string& field) {
  try {
    return std::stoi(token);
  } catch (const std::exception&) {
    throw FileIOException("Nilai " + field + " tidak valid pada " + type +
                          " baris " + std::to_string(lineNo) + ": " + token + "\n");
  }
}

void requireTokenCount(const std::vector<std::string>& tokens, std::size_t expected,
                       const std::string& type, int lineNo) {
  if (tokens.size() != expected) {
    throw FileIOException("Jumlah kolom " + type + " baris " + std::to_string(lineNo) +
                          " tidak valid. Dapat " + std::to_string(tokens.size()) +
                          ", perlu " + std::to_string(expected) + ".\n");
  }
}
}

ConfigHandler::ConfigHandler(string path, string configType)
{
  this->type = configType;
  this->configStream.open(path);
  if (!this->configStream.is_open()) 
  {
    throw FileIOException("Gagal memuat file config: " + path + "\n");
  }
}

// PropertyConfig

PropertyConfig::PropertyConfig(string path) : ConfigHandler(path, "propertyConfig"){}

void PropertyConfig::loadConfig()
{
  string line;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  int lineNo = 1;
  while(getline(this->configStream, line))
  {
    ++lineNo;
    vector<string> tokens = splitTokens(line);
    if (tokens.empty()) {
      continue;
    }

    if (tokens.size() < 7) {
      throw FileIOException("Jumlah kolom propertyConfig baris " + std::to_string(lineNo) + " tidak valid.\n");
    }

    int idProp = parseIntToken(tokens[0], this->type, lineNo, "ID");
    string kode = tokens[1];
    string nama = tokens[2];
    string jenis = tokens[3];
    string warna = tokens[4];
    int hargaLahan = parseIntToken(tokens[5], this->type, lineNo, "HARGA_LAHAN");
    int nilaiGadai = parseIntToken(tokens[6], this->type, lineNo, "NILAI_GADAI");
    vector<int> rent;

    if (jenis == "STREET") {
      requireTokenCount(tokens, 15, this->type, lineNo);
      for (std::size_t i = 7; i < tokens.size(); ++i) {
        rent.push_back(parseIntToken(tokens[i], this->type, lineNo, "STREET_VALUE"));
      }
    } else if (jenis == "RAILROAD" || jenis == "UTILITY") {
      requireTokenCount(tokens, 7, this->type, lineNo);
    } else {
      throw FileIOException("Jenis properti tidak dikenal pada baris " + std::to_string(lineNo) +
                            ": " + jenis + "\n");
    }

    this->propertyConfigs.push_back(make_tuple
    (
      idProp, kode, nama, jenis, warna, hargaLahan, nilaiGadai, rent
    ));

  }
}

vector<tuple<int, string, string, string, string, int, int, vector<int>>> PropertyConfig::getData()
{
  return this->propertyConfigs;
}


// PublicFacilityConfig

RailroadConfig::RailroadConfig(string path) : ConfigHandler(path, "railroadConfig") {}

void RailroadConfig::loadConfig()
{
  string line;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  int lineNo = 1;
  while(getline(this->configStream, line))
  {
    ++lineNo;
    vector<string> tokens = splitTokens(line);
    if (tokens.empty()) {
      continue;
    }
    requireTokenCount(tokens, 2, this->type, lineNo);
    int count = parseIntToken(tokens[0], this->type, lineNo, "JUMLAH_RAILROAD");
    int sewa = parseIntToken(tokens[1], this->type, lineNo, "BIAYASEWA");
    this->railRoadRentTable.insert({count, sewa});
  }
}

map<int,int> RailroadConfig::getRailRoadRentTable() 
{
  return this->railRoadRentTable;
}

// UtilityConfig

UtilityConfig::UtilityConfig(string path) : ConfigHandler(path, "utilityConfig") {}

void UtilityConfig::loadConfig()
{
  string line;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  int lineNo = 1;
  while(getline(this->configStream, line))
  {
    ++lineNo;
    vector<string> tokens = splitTokens(line);
    if (tokens.empty()) {
      continue;
    }
    requireTokenCount(tokens, 2, this->type, lineNo);
    int count = parseIntToken(tokens[0], this->type, lineNo, "JUMLAH_UTILITY");
    int sewa = parseIntToken(tokens[1], this->type, lineNo, "FAKTOR_PENGALI");
    this->utilityMultiplier.insert({count, sewa});
  }
}

map<int,int> UtilityConfig::getUtilityMultiplier()
{
  return this->utilityMultiplier;
}

//TaxConfig

TaxConfig::TaxConfig(string path): ConfigHandler(path, "taxConfig"){}

void TaxConfig::loadConfig()
{
  string line;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  int lineNo = 1;
  while(getline(this->configStream, line))
  {
    ++lineNo;
    vector<string> tokens = splitTokens(line);
    if (tokens.empty()) {
      continue;
    }
    requireTokenCount(tokens, 3, this->type, lineNo);
    int pph = parseIntToken(tokens[0], this->type, lineNo, "PPH_FLAT");
    int persen = parseIntToken(tokens[1], this->type, lineNo, "PPH_PERSENTASE");
    int pbm = parseIntToken(tokens[2], this->type, lineNo, "PBM_FLAT");
    this->taxConfig = make_tuple(pph, persen, pbm);
  }
}

tuple<int, int, int> TaxConfig::getTaxConfig()
{
  return this->taxConfig;
}

//ActionTileConfig

ActionTileConfig::ActionTileConfig(string path): ConfigHandler(path, "actionTileConfig"){}

void ActionTileConfig::loadConfig()
{
  string line;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  int lineNo = 1;
  while(getline(this->configStream, line))
  {
    ++lineNo;
    vector<string> tokens = splitTokens(line);
    if (tokens.empty()) {
      continue;
    }
    requireTokenCount(tokens, 5, this->type, lineNo);
    int id = parseIntToken(tokens[0], this->type, lineNo, "ID");
    string kode = tokens[1];
    string nama = tokens[2];
    string jenisPetak = tokens[3];
    string warna = tokens[4];
    this->actionTileConfig.push_back(make_tuple(id, kode, nama, jenisPetak, warna));
  }
}

vector<tuple<int,string,string,string,string>> ActionTileConfig::getActionTileConfig()
{
  return this->actionTileConfig;
}

//SpecialTileConfig

SpecialTileConfig::SpecialTileConfig(string path): ConfigHandler(path, "specialTileConfig"){}

void SpecialTileConfig::loadConfig()
{
  string line;
  
  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  int lineNo = 1;
  while(getline(this->configStream, line))
  {
    ++lineNo;
    vector<string> tokens = splitTokens(line);
    if (tokens.empty()) {
      continue;
    }
    requireTokenCount(tokens, 2, this->type, lineNo);
    int goSal = parseIntToken(tokens[0], this->type, lineNo, "GO_SALARY");
    int jailFine = parseIntToken(tokens[1], this->type, lineNo, "JAIL_FINE");
    this->specialTileConfig = make_tuple(goSal, jailFine);
  }
}

tuple<int, int> SpecialTileConfig::getSpecialTileConfig()
{
  return this->specialTileConfig;
}

//MiscTileConfig

MiscTileConfig::MiscTileConfig(string path): ConfigHandler(path, "miscConfig"){}

void MiscTileConfig::loadConfig()
{
  string line;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  int lineNo = 1;
  while(getline(this->configStream, line))
  {
    ++lineNo;
    vector<string> tokens = splitTokens(line);
    if (tokens.empty()) {
      continue;
    }
    requireTokenCount(tokens, 2, this->type, lineNo);
    int maxTurn = parseIntToken(tokens[0], this->type, lineNo, "MAX_TURN");
    int saldoAwal = parseIntToken(tokens[1], this->type, lineNo, "SALDO_AWAL");
    this->miscTileConfig= make_tuple(maxTurn, saldoAwal);
  }
}

tuple<int, int> MiscTileConfig::getMiscTileConfig()
{
  return this-> miscTileConfig;
}
