#include "utils/TaxTile.hpp"
#include <iostream>

TaxTile::TaxTile(int id, std::string code, std::string name, TaxType type)
    : ActionTile(id, code, name), taxType(type) {}

void TaxTile::onLand(Player* player, Game* game) {
    (void) player;
    (void) game;
    if (taxType == PPH) {
        std::cout << "Anda mendarat di PPH.\n";
        std::cout << "1. Bayar flat\n2. Bayar persentase\n";

        int choice;
        std::cin >> choice;

        if (choice == 1) {
            // player->debit(flat)
        } else {
            // int tax = player->getTotalWealth() * 0.1
            // player->debit(tax)
        }
    } else {
        std::cout << "Anda mendarat di PBM.\n";
        // player->debit(flat PBM)
    }
}