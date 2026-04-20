#ifndef RAILROAD_TILE_HPP
#define RAILROAD_TILE_HPP

#include "PropertyTile.hpp"
#include <map>

class RailroadTile : public PropertyTile {
private:
    std::map<int, int> rentTable;

public:
    RailroadTile(
        int id,
        std::string code,
        std::string name,
        int mortgageValue,
        std::map<int, int> rentTable = {{1, 25}, {2, 50}, {3, 100}, {4, 200}}
    );

    int calculateRent(Player* visitor, Game* game) override;
    int calculateRentForRailroadCount(int ownedRailroadCount) const;
    int getLandPrice() const override;
    const std::map<int, int>& getRentTable() const;
    void setRentTable(const std::map<int, int>& newRentTable);

    void acquireAutomatically(Player* player);
};

#endif