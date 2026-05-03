#include "utils/StreetTile.hpp"
#include "core/Game.hpp"
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace {
std::vector<const StreetTile*> collectColorGroup(Game* game, const std::string& colorGroup) {
    std::vector<const StreetTile*> group;
    if (game == nullptr) {
        return group;
    }

    for (Tile* tile : game->getBoard().getTiles()) {
        const StreetTile* street = dynamic_cast<const StreetTile*>(tile);
        if (street != nullptr && street->getColorGroup() == colorGroup) {
            group.push_back(street);
        }
    }

    return group;
}
}

StreetTile::StreetTile(int id, std::string code, std::string name,
                       std::string colorGroup, int landPrice,
                       int houseCost, int hotelCost,
                       std::vector<int> rentTable, int mortgageValue)
    : PropertyTile(id, code, name, mortgageValue),
      colorGroup(colorGroup),
      landPrice(landPrice),
      houseBuildCost(houseCost),
      hotelBuildCost(hotelCost),
      rentTable(rentTable),
      houseCount(0),
      hasHotel(false) {}

int StreetTile::calculateRent(Player* visitor, Game* game) {
    (void) visitor;

    if (isMortgaged() || rentTable.empty()) {
        return 0;
    }

    int level = getBuildingLevel();
    int rent = rentTable.at(level);

    if (level == 0 && hasMonopoly(game)) {
        rent *= 2;
    }

    return rent * getFestivalMultiplier();
}

int StreetTile::getBuildingLevel() const {
    if (hasHotel) return 5;
    return houseCount;
}

int StreetTile::getLandPrice() const {
    return landPrice;
}

void StreetTile::buildHouse() {
    if (hasHotel || houseCount >= 4) {
        throw std::logic_error("Cannot build another house on this street.");
    }

    ++houseCount;
}

void StreetTile::buildHotel() {
    if (hasHotel || houseCount != 4) {
        throw std::logic_error("A hotel can only be built after four houses.");
    }

    houseCount = 0;
    hasHotel = true;
}

void StreetTile::demolish() {
    houseCount = 0;
    hasHotel = false;
}

bool StreetTile::hasMonopoly(Game* game) {
    return hasMonopoly(collectColorGroup(game, colorGroup));
}

bool StreetTile::canBuildHouse(Game* game) {
    return canBuildHouse(collectColorGroup(game, colorGroup));
}

bool StreetTile::canBuildHotel(Game* game) {
    return canBuildHotel(collectColorGroup(game, colorGroup));
}

bool StreetTile::hasMonopoly(const std::vector<const StreetTile*>& group) const {
    if (owner == nullptr || group.empty()) {
        return false;
    }

    for (const StreetTile* street : group) {
        if (street == nullptr) {
            return false;
        }
        if (street->getColorGroup() != colorGroup || street->getOwner() != owner) {
            return false;
        }
    }

    return true;
}

bool StreetTile::canBuildHouse(const std::vector<const StreetTile*>& group) const {
    if (!hasMonopoly(group) || isMortgaged() || hasHotel || houseCount >= 4) {
        return false;
    }

    int minHouseCount = houseCount;
    for (const StreetTile* street : group) {
        if (street == nullptr || street->isMortgaged() || street->hasHotelBuilt()) {
            return false;
        }
        minHouseCount = std::min(minHouseCount, street->getHouseCount());
    }

    return houseCount <= minHouseCount;
}

bool StreetTile::canBuildHotel(const std::vector<const StreetTile*>& group) const {
    if (!hasMonopoly(group) || isMortgaged() || hasHotel || houseCount != 4) {
        return false;
    }

    for (const StreetTile* street : group) {
        if (street == nullptr || street->isMortgaged()) {
            return false;
        }
        if (!street->hasHotelBuilt() && street->getHouseCount() != 4) {
            return false;
        }
    }

    return true;
}

bool StreetTile::canBeMortgaged(Game* game) const {
    return canBeMortgaged(collectColorGroup(game, colorGroup));
}

bool StreetTile::canBeMortgaged(const std::vector<const StreetTile*>& group) const {
    if (status != OWNED) {
        return false;
    }

    for (const StreetTile* street : group) {
        if (street != nullptr && street->getColorGroup() == colorGroup && street->getBuildingLevel() > 0) {
            return false;
        }
    }

    return true;
}

int StreetTile::sellBuildingsToBank() {
    int total = (houseCount * houseBuildCost + (hasHotel ? hotelBuildCost : 0)) / 2;
    houseCount = 0;
    hasHotel = false;
    return total;
}

int StreetTile::getHouseCount() const {
    return houseCount;
}

bool StreetTile::hasHotelBuilt() const {
    return hasHotel;
}

std::string StreetTile::getColorGroup() const {
    return colorGroup;
}

int StreetTile::getHouseBuildCost() const {
    return houseBuildCost;
}

int StreetTile::getHotelBuildCost() const {
    return hotelBuildCost;
}

const std::vector<int>& StreetTile::getRentTable() const {
    return rentTable;
}
