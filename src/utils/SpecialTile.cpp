#include "utils/SpecialTile.hpp"
#include "core/Game.hpp"
#include "models/Player.hpp"
#include <iostream>

SpecialTile::SpecialTile(int id, std::string code, std::string name, SpecialType type)
    : Tile(id, code, name), type(type) {}

void SpecialTile::onLand(Player* player, Game* game) {
    if (player == nullptr || game == nullptr) {
        return;
    }

    switch (type) {
        case GO:
            std::cout << "GO! Dapat gaji.\n";
            game->awardGoSalary(*player);
            break;

        case JAIL:
            std::cout << "Hanya mampir penjara.\n";
            break;

        case FREE_PARKING:
            std::cout << "Free parking.\n";
            break;

        case GO_TO_JAIL:
            std::cout << "Masuk penjara!\n";
            if (game->getBoard().getJailIndex() >= 0) {
                player->moveTo(game->getBoard().getJailIndex());
            }
            player->enterJail();
            game->getLogger().log(game->getTurnManager().getCurrentTurn(),
                                  player->getUsername(), "PENJARA", "Go To Jail");
            break;
    }
}

bool SpecialTile::isJail() const {
    return this->type == SpecialType::JAIL;
}
