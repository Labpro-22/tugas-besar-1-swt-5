#include "../../include/models/ChanceCard.hpp"

StepbackCard::StepbackCard() : ChanceCard(
    "Mundur 3 petak.",
    [](Player* p, Game* g) {
        // TODO: IMPLEMENT MUNDUR
    }
) {}
RailroadCard::RailroadCard() : ChanceCard(
    "Pergi ke stasiun terdekat.",
    [](Player* p, Game* g) {
        // TODO: IMPLEMENT PERGI KE STASIUN TERDEKAT
    }
) {}
GoToJailCard::GoToJailCard() : ChanceCard(
    "Masuk Penjara.",
    [](Player* p, Game* g) {
        // TODO: PERGI KE PENJARA
    }
) {}
