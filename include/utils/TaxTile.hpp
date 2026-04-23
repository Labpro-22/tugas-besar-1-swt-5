#ifndef TAX_TILE_HPP
#define TAX_TILE_HPP

#include "ActionTile.hpp"

enum TaxType { 
    PPH, 
    PBM 
};

class TaxTile : public ActionTile {
private:
    TaxType taxType;

public:
    TaxTile(int id, std::string code, std::string name, TaxType type);

    void onLand(Player* player, Game* game) override;
};

#endif