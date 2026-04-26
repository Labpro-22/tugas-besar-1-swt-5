#include "../../include/models/CommunityChestCard.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/core/InsufficientFundException.hpp"
#include "../../include/core/BankruptException.hpp"
#include "../../include/core/Game.hpp"
#include "../../include/models/AbilityCard.hpp"

BirthdayCard::BirthdayCard() : CommunityChestCard(
    "Ini adalah hari ulang tahun Anda. Dapatkan M100 dari setiap pemain.",
    [](Player* p, Game* g) {
        for (auto& player : g->getPlayers()) {
            if (player.getId() != p->getId()) {
                try {
                    try {
                        player.pay(100);
                    } catch (InsufficientFundException e) {
                        auto lm = g->getLiquidationManager();
                        // TODO: TRY LIQUIDATION
                    }
                } catch (BankruptException e) {
                    g->getBankruptcyManager().declareBankruptToPlayer(player, *p, *g);
                }
                p->receive(100);
            }
        }
    }
) {}

PayDoctorCard::PayDoctorCard() : CommunityChestCard(
    "Biaya dokter. Bayar M700.",
    [](Player* p, Game* g) {
        try {
            try {
                p->pay(700);
            } catch (InsufficientFundException e) {
                auto lm = g->getLiquidationManager();
                // TODO: TRY LIQUIDATION
            }
        } catch (BankruptException e) {
            g->getBankruptcyManager().declareBankruptToBank(*p, *g);
        }
    }
) {}

NyalegCard::NyalegCard() : CommunityChestCard(
    "Anda mau nyaleg. Bayar M200 kepada setiap pemain.",
    [](Player* p, Game* g) {
        for (auto& player : g->getPlayers()) {
            if (player.getId() != p->getId()) {
                try {
                    try {
                        p->pay(200);
                    } catch (InsufficientFundException e) {
                        auto lm = g->getLiquidationManager();
                        // TODO: TRY LIQUIDATION
                    }
                } catch (BankruptException e) {
                    g->getBankruptcyManager().declareBankruptToPlayer(*p, player, *g);
                }
                player.receive(200);
            }
        }
    }
) {}