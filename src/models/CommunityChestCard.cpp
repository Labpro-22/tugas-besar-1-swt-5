#include "../../include/models/CommunityChestCard.hpp"

BirthdayCard::BirthdayCard() : CommunityChestCard(
    "Ini adalah hari ulang tahun Anda. Dapatkan M100 dari setiap pemain.",
    [](Player* p, Game* g) {
        // TODO: IMPLEMENT BIRTHDAY
    }
) {}

PayDoctorCard::PayDoctorCard() : CommunityChestCard(
    "Biaya dokter. Bayar M700.",
    [](Player* p, Game* g) {
        // TODO: IMPLEMENT DOCTOR FEE
    }
) {}

NyalegCard::NyalegCard() : CommunityChestCard(
    "Anda mau nyaleg. Bayar M200 kepada setiap pemain.",
    [](Player* p, Game* g) {
        // TODO: IMPLEMENT NYALEG FEE
    }
) {}