#include "../../include/core/AuctionManager.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/utils/PropertyTile.hpp"

#include <algorithm>

using namespace std;

namespace {
bool canChallenge(const Player* player, int highestBid) {
    return player != nullptr && !player->isBankrupt() && player->canAfford(highestBid + 1);
}
}

bool AuctionManager::validateBid(const Player& player, int amount) const {
    return auctionActive && amount >= getMinimumBid() && player.canAfford(amount);
}

void AuctionManager::runAuction(PropertyTile& property, vector<Player*>& bidders, Player& trigger, Game& game) {
    (void) game;

    currentProperty = &property;
    activeBidders.clear();

    for (Player* bidder : bidders) {
        if (bidder != nullptr && !bidder->isBankrupt() && bidder->canAfford(0)) {
            activeBidders.push_back(bidder);
        }
    }

    highestBidder = nullptr;
    highestBid = 0;
    hasBid = false;
    currentIndex = 0;
    auctionActive = currentProperty != nullptr && !activeBidders.empty();

    lastWinner = nullptr;
    lastProperty = &property;
    lastWinningBid = 0;
    lastAuctionHadWinner = false;

    if (!auctionActive) {
        currentProperty = nullptr;
        return;
    }

    int triggerIndex = -1;
    for (size_t i = 0; i < activeBidders.size(); i++) {
        if (activeBidders[i] == &trigger) {
            triggerIndex = static_cast<int>(i);
            break;
        }
    }

    currentIndex = triggerIndex >= 0
        ? (triggerIndex + 1) % static_cast<int>(activeBidders.size())
        : 0;

    normalizeCurrentTurn();
}

bool AuctionManager::processAction(const string& action, int amount) {
    if (!auctionActive || activeBidders.empty()) {
        return false;
    }

    normalizeCurrentTurn();
    if (!auctionActive || activeBidders.empty()) {
        return false;
    }

    Player* currentPlayer = activeBidders[static_cast<size_t>(currentIndex)];

    if (action == "PASS") {
        if (!currentPlayerCanPass()) {
            return false;
        }

        activeBidders.erase(activeBidders.begin() + currentIndex);
        if (currentIndex >= static_cast<int>(activeBidders.size()) && !activeBidders.empty()) {
            currentIndex = 0;
        }
        normalizeCurrentTurn();
        return true;
    }

    if (action == "BID") {
        if (!validateBid(*currentPlayer, amount)) {
            return false;
        }

        highestBid = amount;
        highestBidder = currentPlayer;
        hasBid = true;

        if (!activeBidders.empty()) {
            currentIndex = (currentIndex + 1) % static_cast<int>(activeBidders.size());
        }

        normalizeCurrentTurn();
        return true;
    }

    return false;
}

void AuctionManager::normalizeCurrentTurn() {
    if (!auctionActive) {
        return;
    }

    activeBidders.erase(
        remove_if(activeBidders.begin(), activeBidders.end(), [this](Player* bidder) {
            if (bidder == nullptr || bidder->isBankrupt()) {
                return true;
            }

            if (!hasBid) {
                return !bidder->canAfford(0);
            }

            return bidder != highestBidder && !canChallenge(bidder, highestBid);
        }),
        activeBidders.end()
    );

    if (!hasBid) {
        if (activeBidders.empty()) {
            finalizeAuctionNoWinner();
            return;
        }

        if (currentIndex < 0 || currentIndex >= static_cast<int>(activeBidders.size())) {
            currentIndex = 0;
        }
        return;
    }

    bool hasChallenger = false;
    for (Player* bidder : activeBidders) {
        if (bidder != highestBidder && canChallenge(bidder, highestBid)) {
            hasChallenger = true;
            break;
        }
    }

    if (!hasChallenger) {
        finalizeAuction();
        return;
    }

    if (currentIndex < 0 || currentIndex >= static_cast<int>(activeBidders.size())) {
        currentIndex = 0;
    }

    for (size_t i = 0; i < activeBidders.size(); ++i) {
        Player* bidder = activeBidders[static_cast<size_t>(currentIndex)];
        if (bidder != highestBidder && canChallenge(bidder, highestBid)) {
            return;
        }
        currentIndex = (currentIndex + 1) % static_cast<int>(activeBidders.size());
    }

    finalizeAuction();
}

Player* AuctionManager::determineWinner() {
    return highestBidder;
}

void AuctionManager::finalizeAuction() {
    lastProperty = currentProperty;
    lastWinner = highestBidder;
    lastWinningBid = highestBid;
    lastAuctionHadWinner = highestBidder != nullptr && currentProperty != nullptr;

    if (lastAuctionHadWinner) {
        highestBidder->pay(highestBid);
        currentProperty->setOwner(highestBidder);
        highestBidder->addProperty(currentProperty);
    }

    auctionActive = false;
    currentProperty = nullptr;
    highestBidder = nullptr;
    activeBidders.clear();
    currentIndex = 0;
    hasBid = false;
}

void AuctionManager::finalizeAuctionNoWinner() {
    lastProperty = currentProperty;
    lastWinner = nullptr;
    lastWinningBid = 0;
    lastAuctionHadWinner = false;

    auctionActive = false;
    currentProperty = nullptr;
    highestBidder = nullptr;
    activeBidders.clear();
    currentIndex = 0;
    hasBid = false;
}

bool AuctionManager::isAuctionActive() const { return auctionActive; }

Player* AuctionManager::getCurrentTurnPlayer() const {
    if (!auctionActive || activeBidders.empty() || currentIndex < 0 ||
        currentIndex >= static_cast<int>(activeBidders.size())) {
        return nullptr;
    }

    return activeBidders[static_cast<size_t>(currentIndex)];
}

int AuctionManager::getHighestBid() const { return highestBid; }
PropertyTile* AuctionManager::getCurrentProperty() const { return currentProperty; }
Player* AuctionManager::getHighestBidder() const { return highestBidder; }
Player* AuctionManager::getLastWinner() const { return lastWinner; }
PropertyTile* AuctionManager::getLastProperty() const { return lastProperty; }
int AuctionManager::getLastWinningBid() const { return lastWinningBid; }
bool AuctionManager::getLastAuctionHadWinner() const { return lastAuctionHadWinner; }
int AuctionManager::getActiveBidderCount() const { return static_cast<int>(activeBidders.size()); }
int AuctionManager::getMinimumBid() const { return hasBid ? highestBid + 1 : 0; }

bool AuctionManager::currentPlayerCanBid() const {
    Player* current = getCurrentTurnPlayer();
    return current != nullptr && validateBid(*current, getMinimumBid());
}

bool AuctionManager::currentPlayerCanPass() const {
    return getCurrentTurnPlayer() != nullptr && (hasBid || activeBidders.size() > 1U);
}
