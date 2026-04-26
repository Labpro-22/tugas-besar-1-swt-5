#include "utils/RailroadTile.hpp"
#include "../../include/models/Player.hpp"

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
    Player * owner = this->owner;
    auto props = owner->getOwnedProperties();
    int count = 0;
    for (auto prop : props) {
        RailroadTile* rail = dynamic_cast<RailroadTile*>(prop);
        if (rail != nullptr) {
            count++;
        }
    }
    return calculateRentForRailroadCount(count) * getFestivalMultiplier();
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