#include "../../include/data-layer/ConfigHandler.hpp"

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
  string line, token;
  int idProp, hargaLahan, nilaiGadai, upgRmh, upgHtl, ren0, ren1, ren2, ren3, ren4, ren5;
  string kode, nama, jenis, warna;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  while(getline(this->configStream, line))
  {
    istringstream iss(line);
    int ctr = 0;
    while (iss >> token)
    {
      switch (ctr) {
        case 0: idProp = stoi(token); break;
        case 1: kode = token; break;
        case 2: jenis = token; break;
        case 3: warna = token; break;
        case 4: hargaLahan = stoi(token); break;
        case 5: nilaiGadai = stoi(token); break;
        case 6: upgRmh = stoi(token); break;
        case 7: upgHtl = stoi(token); break;
        case 8: ren0 = stoi(token); break;
        case 9: ren1 = stoi(token); break;
        case 10: ren2 = stoi(token); break;
        case 11: ren3 = stoi(token); break;
        case 12: ren4 = stoi(token); break;
        case 13: ren5 = stoi(token); break;
        default:
          throw FileIOException("Jumlah input line untuk " + this->type + " tidak valid.\n");
          break;
      }
      ctr++;
    }

    vector<int> rent = {upgRmh, upgHtl, ren0, ren1, ren2, ren3, ren4, ren5};
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
  string line, token;
  int count, sewa;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  while(getline(this->configStream, line))
  {
    istringstream iss(line);
    int ctr = 0;
    while (iss >> token)
    {
      switch (ctr) {
        case 0: count = stoi(token); break;
        case 1: sewa = stoi(token); break;
        default:
          throw FileIOException("Jumlah input line untuk " + this->type + " tidak valid.\n");
          break;
      }
      ctr++;
    }
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
  string line, token;
  int count, sewa;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  while(getline(this->configStream, line))
  {
    istringstream iss(line);
    int ctr = 0;
    while (iss >> token)
    {
      switch (ctr) {
        case 0: count = stoi(token); break;
        case 1: sewa = stoi(token); break;
        default:
          throw FileIOException("Jumlah input line untuk " + this->type + " tidak valid.\n");
          break;
      }
      ctr++;
    }
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
  string line, token;
  int pph, persen, pbm;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  while(getline(this->configStream, line))
  {
    istringstream iss(line);
    int ctr = 0;
    while (iss >> token)
    {
      switch (ctr) {
        case 0: pph = stoi(token); break;
        case 1: persen = stoi(token); break;
        case 2: pbm = stoi(token); break;
        default:
          throw FileIOException("Jumlah input line untuk " + this->type + " tidak valid.\n");
          break;
      }
      ctr++;
    }
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
  string line, token, kode, nama, jenisPetak, warna;
  int id;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  while(getline(this->configStream, line))
  {
    istringstream iss(line);
    int ctr = 0;
    while (iss >> token)
    {
      switch (ctr) {
        case 0: id = stoi(token); break;
        case 1: kode= (token); break;
        case 2: nama = (token); break;
        case 3: jenisPetak = (token); break;
        case 4: warna = (token); break;
        default:
          throw FileIOException("Jumlah input line untuk " + this->type + " tidak valid.\n");
          break;
      }
      ctr++;
    }
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
  string line, token;
  int goSal, jailFine;
  
  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  while(getline(this->configStream, line))
  {
    istringstream iss(line);
    int ctr = 0;
    while (iss >> token)
    {
      switch (ctr) {
        case 0: goSal = stoi(token); break;
        case 1: jailFine = stoi(token); break;
        default:
          throw FileIOException("Jumlah input line untuk " + this->type + " tidak valid.\n");
          break;
      }
      ctr++;
    }
    this->specialTileConfig = make_tuple(goSal, jailFine);
  }
}

tuple<int, int> SpecialTileConfig::getSpecialTileConfig()
{
  return this->specialTileConfig;
}

//MiscTileConfig

MiscTileConfig::MiscTileConfig(string path): ConfigHandler(path, "taxConfig"){}

void MiscTileConfig::loadConfig()
{
  string line, token;
  int maxTurn, saldoAwal;

  if (!getline(this->configStream, line)) throw FileIOException("File input untuk " + this->type + " tidak valid.\n");

  while(getline(this->configStream, line))
  {
    istringstream iss(line);
    int ctr = 0;
    while (iss >> token)
    {
      switch (ctr) {
        case 0: maxTurn = stoi(token); break;
        case 1: saldoAwal = stoi(token); break;
        default:
          throw FileIOException("Jumlah input line untuk " + this->type + " tidak valid.\n");
          break;
      }
      ctr++;
    }
    this->miscTileConfig= make_tuple(maxTurn, saldoAwal);
  }
}

tuple<int, int> MiscTileConfig::getMiscTileConfig()
{
  return this-> miscTileConfig;
}
