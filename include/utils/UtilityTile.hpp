#ifndef UTILITY_TILE_HPP
#define UTILITY_TILE_HPP

#include "PropertyTile.hpp"
#include <map>

class UtilityTile : public PropertyTile {
private:
    std::map<int, int> multiplierTable;

public:
    UtilityTile(
        int id,
        std::string code,
        std::string name,
        int mortgageValue,
        std::map<int, int> multiplierTable = {{1, 4}, {2, 10}}
    );

    int calculateRent(Player* visitor, Game* game) override;
    int calculateRentFromDice(int diceTotal, int ownedUtilityCount) const;
    int getLandPrice() const override;
    const std::map<int, int>& getMultiplierTable() const;
    void setMultiplierTable(const std::map<int, int>& newMultiplierTable);

    void acquireAutomatically(Player* player);
};

#endif