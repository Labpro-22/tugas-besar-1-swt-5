#include "../../include/models/CommunityChestCard.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/core/InsufficientFundException.hpp"
#include "../../include/core/BankruptException.hpp"
#include "../../include/core/Game.hpp"
#include "../../include/models/AbilityCard.hpp"

BirthdayCard::BirthdayCard() : CommunityChestCard(
    "Ini adalah hari ulang tahun Anda. Dapatkan M100 dari setiap pemain.",
    [](Player* p, Game* g) {
        for (Player& player : g->getPlayers()) {
            if (player.getId() != p->getId()) {
                g->payPlayerOrBankrupt(player, *p, 100, "Kartu ulang tahun");
            }
        }
    }
) {}

PayDoctorCard::PayDoctorCard() : CommunityChestCard(
    "Biaya dokter. Bayar M700.",
    [](Player* p, Game* g) {
        g->payBankOrBankrupt(*p, 700, "Biaya dokter");
    }
) {}

NyalegCard::NyalegCard() : CommunityChestCard(
    "Anda mau nyaleg. Bayar M200 kepada setiap pemain.",
    [](Player* p, Game* g) {
        for (Player& player : g->getPlayers()) {
            if (player.getId() != p->getId()) {
                g->payPlayerOrBankrupt(*p, player, 200, "Kartu nyaleg");
            }
        }
    }
) {}
