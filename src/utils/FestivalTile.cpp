#include "utils/FestivalTile.hpp"
#include <iostream>

FestivalTile::FestivalTile(int id, std::string code, std::string name) : ActionTile(id, code, name) {}

void FestivalTile::onLand(Player* player, Game* game) {
    (void) player;
    (void) game;
    std::cout << "Festival! Pilih properti untuk buff.\n";

    // ambil daftar properti dari player
    // misal: vector<PropertyTile*> props = player->getProperties();

    int index;
    std::cin >> index;

    // props[index]->applyFestival();
}