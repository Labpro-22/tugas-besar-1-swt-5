#include "../../include/utils/BoardBuilder.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/utils/RailroadTile.hpp"
#include "../../include/utils/UtilityTile.hpp"
#include "../../include/utils/SpecialTile.hpp"
#include "../../include/utils/CardTile.hpp"
#include "../../include/utils/TaxTile.hpp"
#include "../../include/utils/FestivalTile.hpp"
#include <algorithm>
#include <map>

void BoardBuilder::build(
    Board& board,
    const std::vector<std::tuple<int,std::string,std::string,std::string,std::string,int,int,std::vector<int>>>& propConfigs,
    const std::vector<std::tuple<int,std::string,std::string,std::string,std::string>>& actionConfigs,
    const std::map<int,int>& railroadRent,
    const std::map<int,int>& utilityMult)
{
    // Gabungkan semua tile berdasarkan ID
    struct TileInfo {
        int id;
        std::string code, name, type, color;
        int landPrice = 0, mortgageValue = 0;
        std::vector<int> rentTable;
        bool isProperty = false;
    };

    std::map<int, TileInfo> tileMap;

    // Property tiles
    for (const auto& pc : propConfigs) {
        TileInfo t;
        t.id            = std::get<0>(pc);
        t.code          = std::get<1>(pc);
        t.name          = std::get<2>(pc);
        t.type          = std::get<3>(pc);
        t.color         = std::get<4>(pc);
        t.landPrice     = std::get<5>(pc);
        t.mortgageValue = std::get<6>(pc);
        t.rentTable     = std::get<7>(pc);
        t.isProperty    = true;
        tileMap[t.id]   = t;
    }

    // Action tiles (special, card, tax, festival)
    for (const auto& ac : actionConfigs) {
        TileInfo t;
        t.id   = std::get<0>(ac);
        t.code = std::get<1>(ac);
        t.name = std::get<2>(ac);
        t.type = std::get<3>(ac);
        t.color= std::get<4>(ac);
        t.isProperty = false;
        tileMap[t.id] = t;
    }

    // Sort by ID dan masukkan ke board
    std::vector<int> sortedIds;
    for (const auto& kv : tileMap) sortedIds.push_back(kv.first);
    std::sort(sortedIds.begin(), sortedIds.end());

    for (int id : sortedIds) {
        const TileInfo& t = tileMap[id];

        if (t.isProperty) {
            if (t.type == "STREET") {
                // rentTable: [rent_l0..l5], houseBuildCost dan hotelBuildCost ada di index 6,7 dari propConfig tapi sudah di-extract
                // Di property.txt format: ... UPG_RUMAH UPG_HT RENT_L0..L5
                // tapi di Config tuple format: (id, code, name, type, color, landPrice, mortgageValue, rentTable)
                // rentTable sudah berisi semua rent values
                // Kita perlu houseBuildCost dan hotelBuildCost — tapi di tuple tidak ada field terpisah...
                // Berdasarkan property.txt, UPG_RUMAH dan UPG_HT ada sebelum rent values
                // Tapi Config menyimpannya di rentTable[0..N]
                // Dari property.txt: UPG_RUMAH=rentTable[0], UPG_HT=rentTable[1], RENT_L0..L5=rentTable[2..7]
                // Wait — mari cek ulang format Config tuple
                
                // Config stores: (id, code, name, type, color, landPrice, mortgageValue, {UPG_RUMAH, UPG_HT, RENT_L0..L5})
                // So rentTable = {upgRumah, upgHotel, rent0, rent1, rent2, rent3, rent4, rent5}
                int houseCost = t.rentTable.size() > 0 ? t.rentTable[0] : 50;
                int hotelCost = t.rentTable.size() > 1 ? t.rentTable[1] : 50;
                std::vector<int> rents;
                for (size_t i = 2; i < t.rentTable.size(); ++i) rents.push_back(t.rentTable[i]);

                board.addTile(new StreetTile(id, t.code, t.name, t.color,
                                              t.landPrice, houseCost, hotelCost,
                                              rents, t.mortgageValue));
            }
            else if (t.type == "RAILROAD") {
                board.addTile(new RailroadTile(id, t.code, t.name, t.mortgageValue, railroadRent));
            }
            else if (t.type == "UTILITY") {
                board.addTile(new UtilityTile(id, t.code, t.name, t.mortgageValue, utilityMult));
            }
        }
        else {
            // Action tiles
            if (t.type == "SPESIAL") {
                SpecialType st = GO;
                if (t.code == "GO")  st = GO;
                else if (t.code == "PEN") st = JAIL;
                else if (t.code == "BBP") st = FREE_PARKING;
                else if (t.code == "PPJ") st = GO_TO_JAIL;
                board.addTile(new SpecialTile(id, t.code, t.name, st));
            }
            else if (t.type == "KARTU") {
                CardDeckType ct = (t.code == "KSP") ? CHANCE : COMMUNITY_CHEST;
                board.addTile(new CardTile(id, t.code, t.name, ct));
            }
            else if (t.type == "PAJAK") {
                TaxType tt = (t.code == "PBM") ? PBM : PPH;
                board.addTile(new TaxTile(id, t.code, t.name, tt));
            }
            else if (t.type == "FESTIVAL") {
                board.addTile(new FestivalTile(id, t.code, t.name));
            }
            else {
                // Fallback: special tile
                board.addTile(new SpecialTile(id, t.code, t.name, FREE_PARKING));
            }
        }
    }
}
