#include "utils/UtilityTile.hpp"
#include "core/Game.hpp"
#include "models/Player.hpp"

UtilityTile::UtilityTile(
    int id,
    std::string code,
    std::string name,
    int mortgageValue,
    std::map<int, int> multiplierTable
) : PropertyTile(id, code, name, mortgageValue), multiplierTable(multiplierTable) {}

int UtilityTile::calculateRent(Player* visitor, Game* game) {
    (void) visitor;
    if (isMortgaged()) {
        return 0;
    }

    int ownedUtilityCount = 1;
    if (owner != nullptr) {
        ownedUtilityCount = 0;
        for (PropertyTile* prop : owner->getOwnedProperties()) {
            if (dynamic_cast<UtilityTile*>(prop) != nullptr && !prop->isMortgaged()) {
                ownedUtilityCount++;
            }
        }
    }

    int diceTotal = (game != nullptr) ? game->getLastDiceTotal() : 0;
    return calculateRentFromDice(diceTotal, ownedUtilityCount) * getFestivalMultiplier();
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
