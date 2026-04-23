#ifndef SPECIAL_TILE_HPP
#define SPECIAL_TILE_HPP

#include "Tile.hpp"

enum SpecialType { 
    GO, 
    JAIL, 
    FREE_PARKING, 
    GO_TO_JAIL 
};

class SpecialTile : public Tile {
private:
    SpecialType type;

public:
    SpecialTile(int id, std::string code, std::string name, SpecialType type);

    void onLand(Player* player, Game* game) override;
    bool isJail() const;
};

#endif