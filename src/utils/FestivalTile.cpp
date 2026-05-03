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

    game->requestFestivalSelection(*player);
}
