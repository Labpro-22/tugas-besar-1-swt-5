#include "utils/RailroadTile.hpp"
#include "models/Player.hpp"
#include <algorithm>

RailroadTile::RailroadTile(
    int id,
    std::string code,
    std::string name,
    int mortgageValue,
    std::map<int, int> rentTable
) : PropertyTile(id, code, name, mortgageValue), rentTable(rentTable) {}

int RailroadTile::calculateRent(Player* visitor, Game* game) {
    (void) visitor;
    (void) game;
    int ownedRailroadCount = 1;
    if (owner != nullptr) {
        ownedRailroadCount = 0;
        for (PropertyTile* prop : owner->getOwnedProperties()) {
            if (dynamic_cast<RailroadTile*>(prop) != nullptr && !prop->isMortgaged()) {
                ownedRailroadCount++;
            }
        }
    }
    return calculateRentForRailroadCount(ownedRailroadCount) * getFestivalMultiplier();
}

int RailroadTile::calculateRentForRailroadCount(int ownedRailroadCount) const {
    if (rentTable.empty()) {
        return 0;
    }

    auto it = rentTable.find(ownedRailroadCount);
    if (it != rentTable.end()) {
        return it->second;
    }

    return rentTable.rbegin()->second;
}

int RailroadTile::getLandPrice() const {
    return 0;
}

const std::map<int, int>& RailroadTile::getRentTable() const {
    return rentTable;
}

void RailroadTile::setRentTable(const std::map<int, int>& newRentTable) {
    rentTable = newRentTable;
}

void RailroadTile::acquireAutomatically(Player* player) {
    setOwner(player);
}
