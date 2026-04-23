#include "utils/CardTile.hpp"
#include <iostream>

CardTile::CardTile(int id, std::string code, std::string name, CardDeckType type) : ActionTile(id, code, name), deckType(type) {}

void CardTile::onLand(Player* player, Game* game) {
    (void) player;
    (void) game;
    std::cout << "Mengambil kartu...\n";

    if (deckType == CHANCE) {
        std::cout << "Kartu Kesempatan!\n";
    } else {
        std::cout << "Dana Umum!\n";
    }

    // game->getCardManager()->draw(deckType)->apply(player, game);
}