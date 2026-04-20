#ifndef ACTION_TILE_HPP
#define ACTION_TILE_HPP

#include "Tile.hpp"

class ActionTile : public Tile {
public:
    ActionTile(int id, std::string code, std::string name);
    virtual void onLand(Player* player, Game* game) = 0;
};

#endif