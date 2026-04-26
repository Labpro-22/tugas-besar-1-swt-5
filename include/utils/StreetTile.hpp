#ifndef STREET_TILE_HPP
#define STREET_TILE_HPP

#include "PropertyTile.hpp"
#include <string>
#include <vector>

class StreetTile : public PropertyTile {
private:
    std::string colorGroup;
    int landPrice;

    int houseBuildCost;
    int hotelBuildCost;

    std::vector<int> rentTable;

    int houseCount;
    bool hasHotel;

public:
    StreetTile(int id, std::string code, std::string name,
               std::string colorGroup, int landPrice,
               int houseCost, int hotelCost,
               std::vector<int> rentTable, int mortgageValue);

    int calculateRent(Player* visitor, Game* game) override;
    int getLandPrice() const override;

    bool hasMonopoly(Game* game);
    bool canBuildHouse(Game* game);
    bool canBuildHotel(Game* game);
    bool hasMonopoly(const std::vector<const StreetTile*>& group) const;
    bool canBuildHouse(const std::vector<const StreetTile*>& group) const;
    bool canBuildHotel(const std::vector<const StreetTile*>& group) const;
    bool canBeMortgaged(Game* game) const override;
    bool canBeMortgaged(const std::vector<const StreetTile*>& group) const;

    void buildHouse();
    void buildHotel();
    int sellBuildingsToBank();

    int getBuildingLevel() const;
    int getHouseCount() const;
    bool hasHotelBuilt() const;
    std::string getColorGroup() const;
    int getHouseBuildCost() const;
    int getHotelBuildCost() const;
    const std::vector<int>& getRentTable() const;
    void cetakAkta() const;

};

#endif