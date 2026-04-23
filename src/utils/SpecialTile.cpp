#include "utils/SpecialTile.hpp"
#include <iostream>

SpecialTile::SpecialTile(int id, std::string code, std::string name, SpecialType type)
    : Tile(id, code, name), type(type) {}

void SpecialTile::onLand(Player* player, Game* game) {
    (void) player;
    (void) game;
    switch (type) {
        case GO:
            std::cout << "GO! Dapat gaji.\n";
            // game->awardGoSalary(player);
            break;

        case JAIL:
            std::cout << "Hanya mampir penjara.\n";
            break;

        case FREE_PARKING:
            std::cout << "Free parking.\n";
            break;

        case GO_TO_JAIL:
            std::cout << "Masuk penjara!\n";
            // player->enterJail();
            break;
    }
}

bool SpecialTile::isJail() const {
    return this->type == SpecialType::JAIL;
}