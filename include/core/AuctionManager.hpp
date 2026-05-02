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
    Player* lastWinner = nullptr;
    PropertyTile* lastProperty = nullptr;
    int highestBid = 0;
    int lastWinningBid = 0;
    
    int currentIndex = 0;
    bool hasBid = false;
    bool auctionActive = false;
    bool lastAuctionHadWinner = false;

    void normalizeCurrentTurn();
    void finalizeAuctionNoWinner();

public:
    AuctionManager() = default;
    ~AuctionManager() = default;

    void runAuction(PropertyTile& property, vector<Player*>& bidders, Player& trigger, Game& game);
    
    bool processAction(const string& action, int amount = 0);

    bool validateBid(const Player& player, int amount) const;
    Player* determineWinner();
    void finalizeAuction();

    // Getter untuk UI Layer
    bool isAuctionActive() const;
    Player* getCurrentTurnPlayer() const;
    int getHighestBid() const;
    PropertyTile* getCurrentProperty() const;
    Player* getHighestBidder() const;
    Player* getLastWinner() const;
    PropertyTile* getLastProperty() const;
    int getLastWinningBid() const;
    bool getLastAuctionHadWinner() const;
    int getActiveBidderCount() const;
    int getMinimumBid() const;
    bool currentPlayerCanBid() const;
    bool currentPlayerCanPass() const;
};
