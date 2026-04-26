#include "utils/StreetTile.hpp"
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include "../../include/models/Player.hpp"

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
    (void) game;

    if (rentTable.empty()) {
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

bool StreetTile::hasMonopoly(Game* game) {
    (void) game;
    return false;
}

bool StreetTile::canBuildHouse(Game* game) {
    (void) game;
    return owner != nullptr && !isMortgaged() && !hasHotel && houseCount < 4;
}

bool StreetTile::canBuildHotel(Game* game) {
    (void) game;
    return owner != nullptr && !isMortgaged() && !hasHotel && houseCount == 4;
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
        if (street == nullptr || street->hasHotelBuilt()) {
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
        if (street == nullptr || street->hasHotelBuilt() || street->getHouseCount() != 4) {
            return false;
        }
    }

    return true;
}

bool StreetTile::canBeMortgaged(Game* game) const {
    (void) game;
    return status == OWNED && getBuildingLevel() == 0;
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

void StreetTile::cetakAkta() const {
    std::cout << "+================================+" << std::endl;
    std::cout << "|        AKTA KEPEMILIKAN        |" << std::endl;
    std::string info = "[" + getColorGroup() + "] " + name + " (" + code + ")";
    int left = (33 - info.length()) / 2;
    int right = 33 - left;
    std::cout << "|";
    for (size_t i = 0; i < left; i++){std::cout << " ";}
    std::cout << info;
    for (size_t i = 0; i < right; i++){std::cout << " ";}
    std::cout << "|" << std::endl;
    std::cout << "+================================+" << std::endl;
    printf("| Harga Beli        : M%4d      |\n", getLandPrice());
    printf("| Harga Gadai       : M%4d      |\n", getMortgageValue());
    printf("+--------------------------------+\n");
    printf("| Sewa (unimproved) : M%4d      |\n", rentTable[0]);   
    printf("| Sewa (1 rumah)    : M%4d      |\n", rentTable[1]);
    printf("| Sewa (2 rumah)    : M%4d      |\n", rentTable[2]);
    printf("| Sewa (3 rumah)    : M%4d      |\n", rentTable[3]);
    printf("| Sewa (4 rumah)    : M%4d      |\n", rentTable[4]);
    printf("| Sewa (hotel)      : M%4d      |\n", rentTable[5]);
    printf("+================================+\n");
    std::string statusString;
    switch (status)
    {
    case PropertyStatus::MORTGAGED:
        statusString = "MORTGAGED (" + this->getOwner()->getUsername() + ")";
        break;
    case PropertyStatus::BANK:
        statusString = "BANK";
        break;
    case PropertyStatus::OWNED:
        statusString = "OWNED (" + this->getOwner()->getUsername() + ")";
        break;   
    }
    printf("| Status : ");
    printf(statusString.c_str());
    for (int i = 0; i < 20 - statusString.length(); i++) {printf(" ");}
    printf(" |\n");
    printf("+================================+\n");
}