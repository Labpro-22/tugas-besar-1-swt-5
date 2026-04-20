#ifndef CARD_TILE_HPP
#define CARD_TILE_HPP

#include "ActionTile.hpp"

enum CardDeckType { 
    CHANCE, 
    COMMUNITY_CHEST 
};

class CardTile : public ActionTile {
private:
    CardDeckType deckType;

public:
    CardTile(int id, std::string code, std::string name, CardDeckType type);
    
    void onLand(Player* player, Game* game) override;
};

#endif