#pragma once

#include <vector>

class Player;
class PropertyTile;

class TradeToPlayer {
private:
    Player* proposer;
    Player* target;
    std::vector<PropertyTile*> offeredProperties;
    int offeredMoney;
    std::vector<PropertyTile*> requestedProperties;
    int requestedMoney;

public:
    TradeToPlayer(Player* proposer, Player* target, 
                  const std::vector<PropertyTile*>& offeredProps, int offeredMoney,
                  const std::vector<PropertyTile*>& requestedProps, int requestedMoney);
    ~TradeToPlayer() = default;

    Player* getProposer() const;
    Player* getTarget() const;
    std::vector<PropertyTile*> getOfferedProperties() const;
    int getOfferedMoney() const;
    std::vector<PropertyTile*> getRequestedProperties() const;
    int getRequestedMoney() const;
};