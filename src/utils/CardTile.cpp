#include "utils/CardTile.hpp"
#include "core/Game.hpp"
#include "models/ChanceCard.hpp"
#include "models/CommunityChestCard.hpp"
#include "models/Player.hpp"
#include <iostream>

CardTile::CardTile(int id, std::string code, std::string name, CardDeckType type) : ActionTile(id, code, name), deckType(type) {}

void CardTile::onLand(Player* player, Game* game) {
    if (player == nullptr || game == nullptr) {
        return;
    }

    std::cout << "Mengambil kartu...\n";

    if (deckType == CHANCE) {
        std::cout << "Kartu Kesempatan!\n";
        ChanceCard* card = game->getCardManager().drawChanceCard();
        if (card != nullptr) {
            game->getLogger().log(game->getTurnManager().getCurrentTurn(),
                                  player->getUsername(), "KARTU", card->getDescription());
            card->apply(player, game);
        }
    } else {
        std::cout << "Dana Umum!\n";
        CommunityChestCard* card = game->getCardManager().drawCommunityCard();
        if (card != nullptr) {
            game->getLogger().log(game->getTurnManager().getCurrentTurn(),
                                  player->getUsername(), "KARTU", card->getDescription());
            card->apply(player, game);
        }
    }
}
