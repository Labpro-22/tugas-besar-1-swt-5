#pragma once

#include <vector>
#include <string>

class Game;
class Player;
class PropertyTile;

using namespace std;

class AuctionManager {
private:
    PropertyTile* currentProperty = nullptr;
    vector<Player*> activeBidders;
    Player* highestBidder = nullptr;
    int highestBid = 0;
    
    int currentIndex = 0;
    int consecutivePasses = 0;
    int requiredPasses = 0;
    bool hasBid = false;
    bool auctionActive = false;

public:
    AuctionManager() = default;
    ~AuctionManager() = default;

    void runAuction(PropertyTile& property, vector<Player*>& bidders, Player& trigger, Game& game);
    
    void processAction(const string& action, int amount = 0);

    bool validateBid(const Player& player, int amount) const;
    Player* determineWinner();
    void finalizeAuction();

    // Getter untuk UI Layer
    bool isAuctionActive() const;
    Player* getCurrentTurnPlayer() const;
    int getHighestBid() const;
    PropertyTile* getCurrentProperty() const;
};