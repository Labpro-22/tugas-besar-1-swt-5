#include "utils/UtilityTile.hpp"
#include "../../include/models/Dice.hpp"
#include "../../include/core/Game.hpp"

UtilityTile::UtilityTile(
    int id,
    std::string code,
    std::string name,
    int mortgageValue,
    std::map<int, int> multiplierTable
) : PropertyTile(id, code, name, mortgageValue), multiplierTable(multiplierTable) {}

int UtilityTile::calculateRent(Player* visitor, Game* game) {
    (void) visitor;
    (void) game;
    Player* owner = this->owner;
    auto props = owner->getOwnedProperties();
    int count = 0;
    for (auto prop : props) {
        UtilityTile* rail = dynamic_cast<UtilityTile*>(prop);
        if (rail != nullptr) {
            count++;
        }
    }
    auto [dice1, dice2] = game->getDice().getResult();
    return calculateRentFromDice(dice1 + dice2, count) * getFestivalMultiplier();
}

int UtilityTile::calculateRentFromDice(int diceTotal, int ownedUtilityCount) const {
    if (multiplierTable.empty()) {
        return 0;
    }

    auto it = multiplierTable.find(ownedUtilityCount);
    if (it == multiplierTable.end()) {
        it = std::prev(multiplierTable.end());
    }

    return diceTotal * it->second;
}

int UtilityTile::getLandPrice() const {
    return 0;
}

const std::map<int, int>& UtilityTile::getMultiplierTable() const {
    return multiplierTable;
}

void UtilityTile::setMultiplierTable(const std::map<int, int>& newMultiplierTable) {
    multiplierTable = newMultiplierTable;
}

void UtilityTile::acquireAutomatically(Player* player) {
    setOwner(player);
}