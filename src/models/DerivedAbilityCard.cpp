#include "../../include/models/DerivedAbilityCard.hpp"

MoveCard::MoveCard() : AbilityCard("MoveCard", "Maju x petak.") {}
void MoveCard::setup() {
    std::mt19937 randomizer(std::random_device{}());
    std::uniform_int_distribution picker(1, 12);
    steps = picker(randomizer);
    description = "Maju " + std::to_string(steps) + " petak.";
}
void MoveCard::use(Player* target, Game* game) {
    // TODO: MOVE PLAYER
}

DiscountCard::DiscountCard() : AbilityCard("DiscountCard", "Dapatkan diskon selama 1 giliran.") {}
void DiscountCard::setup() {
    std::mt19937 randomizer(std::random_device{}());
    std::uniform_int_distribution picker(1, 100);
    percentage = picker(randomizer);
    description = "Dapatkan diskon " + std::to_string(percentage) + "%% selama 1 giliran.";
}
void DiscountCard::use(Player* target, Game* game) {
    // TODO: APPLY DISCOUNT
}

ShieldCard::ShieldCard() : AbilityCard("ShieldCard", "Terlindung dari tagihan sewa maupun sanksi apapun yang merugikan selama 1 giliran.") {}
void ShieldCard::use(Player* target, Game* game) {
    // TODO: APPLY SHIELD
}

TeleportCard::TeleportCard() : AbilityCard("TeleportCard", "Bebas berpindah ke petak manapun di atas papan permainan.") {}

void TeleportCard::use(Player* target, Game* game) {
    // TODO: TELEPORT
}


LassoCard::LassoCard() : AbilityCard("LassoCard", "Tarik satu pemain lain di depanmu ke posisimu.") {}
void LassoCard::use(Player* target, Game* game) {
    // TODO: PULL OTHER PLAYER
}


DemolitionCard::DemolitionCard() : AbilityCard("DemolitionCard", "Hancurkan satu properti milik pemain lain.") {}
void DemolitionCard::use(Player* target, Game* game) {
    // TODO: DEMOLISH
}