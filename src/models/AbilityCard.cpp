#include "../../include/models/AbilityCard.hpp"
#include "../../include/models/Player.hpp"

bool AbilityCard::canUse(Player* target, Game* game) const {
    (void)game;
    if (target == nullptr) return false;
    if (target->hasUsedAbilityThisTurn()) return false;
    return true;
}

std::string AbilityCard::serialize() const {
    return name + " " + description + " 0";
}
