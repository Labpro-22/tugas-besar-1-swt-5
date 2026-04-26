#include "utils/TaxTile.hpp"
#include "core/Game.hpp"
#include "models/Player.hpp"
#include <iostream>

TaxTile::TaxTile(int id, std::string code, std::string name, TaxType type)
    : ActionTile(id, code, name), taxType(type) {}

void TaxTile::onLand(Player* player, Game* game) {
    if (player == nullptr || game == nullptr) {
        return;
    }

    if (taxType == TAX_PPH) {
        const int flatTax = game->getConfig().getTaxConfig(PPH);
        const int percent = game->getConfig().getTaxConfig(PERSEN);

        // UI selection belum tersedia di core loop ini. Default aman: pilih flat
        // sebelum total wealth dihitung, sesuai aturan PPH.
        (void) percent;
        std::cout << player->getUsername() << " membayar PPH flat M" << flatTax << ".\n";
        game->payBankOrBankrupt(*player, flatTax, "PPH flat");
    } else {
        const int tax = game->getConfig().getTaxConfig(PBM);
        std::cout << player->getUsername() << " membayar PBM M" << tax << ".\n";
        game->payBankOrBankrupt(*player, tax, "PBM");
    }
}
