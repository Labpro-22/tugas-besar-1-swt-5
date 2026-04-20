#ifndef FESTIVAL_TILE_HPP
#define FESTIVAL_TILE_HPP

#include "ActionTile.hpp"

class FestivalTile : public ActionTile {
public:
    FestivalTile(int id, std::string code, std::string name);
    
    void onLand(Player* player, Game* game) override;
};

#endif