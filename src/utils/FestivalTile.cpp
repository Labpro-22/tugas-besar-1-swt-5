#include "utils/FestivalTile.hpp"
#include "utils/PropertyTile.hpp"
#include "core/Game.hpp"
#include "models/Player.hpp"
#include <iostream>

FestivalTile::FestivalTile(int id, std::string code, std::string name) : ActionTile(id, code, name) {}

void FestivalTile::onLand(Player* player, Game* game) {
    if (player == nullptr || game == nullptr) {
        return;
    }

    auto& properties = player->getOwnedProperties();
    if (properties.empty()) {
        std::cout << "Festival tidak aktif karena " << player->getUsername()
                  << " belum memiliki properti.\n";
        game->getLogger().log(game->getTurnManager().getCurrentTurn(),
                              player->getUsername(), "FESTIVAL", "Tidak ada properti");
        return;
    }

    PropertyTile* selected = properties.front();
    selected->applyFestival();

    std::cout << "Festival diterapkan ke " << selected->getName()
              << " x" << selected->getFestivalMultiplier()
              << " selama " << selected->getFestivalDuration() << " giliran.\n";
    game->getLogger().log(game->getTurnManager().getCurrentTurn(),
                          player->getUsername(), "FESTIVAL",
                          selected->getName() + " x" + std::to_string(selected->getFestivalMultiplier()));
}
