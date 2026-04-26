#include "../../include/core/TradeManager.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/models/TradeToPlayer.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/core/InvalidActionException.hpp"
#include <algorithm>

using namespace std;

TradeManager::~TradeManager() {
    for (TradeToPlayer* trade : activeTrades) {
        delete trade;
    }
    activeTrades.clear();
}

TradeToPlayer* TradeManager::proposeTrade(Player* proposer, Player* target,
                                          const vector<PropertyTile*>& offeredProps, int offeredMoney,
                                          const vector<PropertyTile*>& requestedProps, int requestedMoney) {
    
    TradeToPlayer* newTrade = new TradeToPlayer(proposer, target, offeredProps, offeredMoney, requestedProps, requestedMoney);
    
    if (!validateTrade(newTrade)) {
        delete newTrade;
        throw InvalidActionException("Penawaran ditolak! Pastikan properti tidak memiliki bangunan dan uang mencukupi.");
    }
    
    activeTrades.push_back(newTrade);
    return newTrade;
}

bool TradeManager::validateTrade(TradeToPlayer* trade) const {
    if (!trade) return false;

    Player* proposer = trade->getProposer();
    Player* target = trade->getTarget();

    if (trade->getOfferedMoney() < 0 || !proposer->canAfford(trade->getOfferedMoney())) return false;
    if (trade->getRequestedMoney() < 0 || !target->canAfford(trade->getRequestedMoney())) return false;

    for (PropertyTile* prop : trade->getOfferedProperties()) {
        if (prop == nullptr || prop->getOwner() != proposer) return false;

        StreetTile* street = dynamic_cast<StreetTile*>(prop);
        if (street != nullptr && street->getHouseCount() > 0) {
            return false; 
        }
    }

    for (PropertyTile* prop : trade->getRequestedProperties()) {
        if (prop == nullptr || prop->getOwner() != target) return false;

        StreetTile* street = dynamic_cast<StreetTile*>(prop);
        if (street != nullptr && street->getHouseCount() > 0) {
            return false; 
        }
    }

    return true;
}

void TradeManager::acceptTrade(TradeToPlayer* trade) {
    if (!validateTrade(trade)) {
        rejectTrade(trade);
        throw InvalidActionException("Penawaran batal otomatis: Uang atau kepemilikan properti telah berubah sejak diajukan!");
    }

    Player* proposer = trade->getProposer();
    Player* target = trade->getTarget();

    if (trade->getOfferedMoney() > 0) {
        proposer->pay(trade->getOfferedMoney());
        target->receive(trade->getOfferedMoney());
    }

    if (trade->getRequestedMoney() > 0) {
        target->pay(trade->getRequestedMoney());
        proposer->receive(trade->getRequestedMoney());
    }

    for (PropertyTile* prop : trade->getOfferedProperties()) {
        proposer->removeProperty(prop);
        prop->setOwner(target); 
        target->addProperty(prop);
    }

    for (PropertyTile* prop : trade->getRequestedProperties()) {
        target->removeProperty(prop);
        prop->setOwner(proposer);
        proposer->addProperty(prop);
    }

    rejectTrade(trade); 
}

void TradeManager::rejectTrade(TradeToPlayer* trade) {
    auto it = find(activeTrades.begin(), activeTrades.end(), trade);
    if (it != activeTrades.end()) {
        activeTrades.erase(it);
        delete trade;
    }
}

vector<TradeToPlayer*> TradeManager::getTradesForPlayer(Player* target) const {
    vector<TradeToPlayer*> result;
    for (TradeToPlayer* trade : activeTrades) {
        if (trade->getTarget() == target) {
            result.push_back(trade);
        }
    }
    return result;
}