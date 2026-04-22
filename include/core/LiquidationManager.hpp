#pragma once

#include <vector>
#include <utility>

using namespace std;    

class Player;
class PropertyTile;

enum class LiquidationAction {
    SELL_TO_BANK,
    MORTGAGE
};

class LiquidationManager {
private:
    int calculatePrice(const Player& player, const PropertyTile* property, LiquidationAction action) const;

public:
    LiquidationManager() = default;
    ~LiquidationManager() = default;

    void liquidateAssets(Player& player, const vector<pair<PropertyTile*, LiquidationAction>>& targetProps);
};