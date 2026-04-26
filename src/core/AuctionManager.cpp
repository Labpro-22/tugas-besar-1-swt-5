#include "../../include/core/AuctionManager.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include <iostream>

using namespace std;

bool AuctionManager::validateBid(const Player& player, int amount) const {
    return player.canAfford(amount);
}

void AuctionManager::runAuction(PropertyTile& property, vector<Player*>& bidders, Player& trigger, Game& game) {
    if (bidders.size() < 2) return;

    this->currentProperty = &property;
    this->activeBidders = bidders;
    this->highestBidder = nullptr;
    this->highestBid = 0;
    this->hasBid = false;
    this->consecutivePasses = 0;
    this->requiredPasses = bidders.size() - 1;
    this->auctionActive = true;

    int triggerIndex = 0;
    for (size_t i = 0; i < bidders.size(); i++) {
        if (bidders[i] == &trigger) {
            triggerIndex = i;
            break;
        }
    }
    this->currentIndex = (triggerIndex + 1) % bidders.size();
}

void AuctionManager::processAction(const string& action, int amount) {
    if (!auctionActive) return;

    Player* currentPlayer = activeBidders[currentIndex];

    if (currentPlayer->isBankrupt()) {
        currentIndex = (currentIndex + 1) % activeBidders.size();
        return;
    }

    if (action == "PASS") {
        if (consecutivePasses == requiredPasses - 1 && !hasBid) {
            return; 
        }
        consecutivePasses++;
    } 
    else if (action == "BID") {
        if (hasBid && amount <= highestBid) return; 
        if (!hasBid && amount < 0) return;        
        if (!validateBid(*currentPlayer, amount)) return; 

        highestBid = amount;
        highestBidder = currentPlayer;
        hasBid = true;
        consecutivePasses = 0;
    }

    currentIndex = (currentIndex + 1) % activeBidders.size();

    if (consecutivePasses >= requiredPasses) {
        finalizeAuction();
    }
}

void AuctionManager::finalizeAuction() {
    if (highestBidder != nullptr && currentProperty != nullptr) {
        highestBidder->pay(highestBid);
        currentProperty->setOwner(highestBidder);
        highestBidder->addProperty(currentProperty);
    }

    auctionActive = false; 
    
    highestBidder = nullptr;
    currentProperty = nullptr;
}

bool AuctionManager::isAuctionActive() const { return auctionActive; }
Player* AuctionManager::getCurrentTurnPlayer() const { return activeBidders[currentIndex]; }
int AuctionManager::getHighestBid() const { return highestBid; }
PropertyTile* AuctionManager::getCurrentProperty() const { return currentProperty; }

// Catatan: perubahan kelas, perhatikan GUI!