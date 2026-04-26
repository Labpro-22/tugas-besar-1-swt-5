#include "../../include/models/TradeToPlayer.hpp"

using namespace std;

TradeToPlayer::TradeToPlayer(Player* proposer, Player* target, 
                             const vector<PropertyTile*>& offeredProps, int offeredMoney,
                             const vector<PropertyTile*>& requestedProps, int requestedMoney)
    : proposer(proposer), target(target), 
      offeredProperties(offeredProps), offeredMoney(offeredMoney),
      requestedProperties(requestedProps), requestedMoney(requestedMoney) {}

Player* TradeToPlayer::getProposer() const { return proposer; }
Player* TradeToPlayer::getTarget() const { return target; }
vector<PropertyTile*> TradeToPlayer::getOfferedProperties() const { return offeredProperties; }
int TradeToPlayer::getOfferedMoney() const { return offeredMoney; }
vector<PropertyTile*> TradeToPlayer::getRequestedProperties() const { return requestedProperties; }
int TradeToPlayer::getRequestedMoney() const { return requestedMoney; }