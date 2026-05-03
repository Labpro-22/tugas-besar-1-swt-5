#pragma once

#include <vector>
#include <string>
#include <utility>

#include "../utils/Board.hpp"
#include "../models/Player.hpp"
#include "TurnManager.hpp"
#include "../models/Dice.hpp"
#include "CardManager.hpp"
#include "AuctionManager.hpp"
#include "BankruptcyManager.hpp"
#include "TradeManager.hpp"
#include "LiquidationManager.hpp"
#include "../utils/Logger.hpp"
#include "../data-layer/Config.hpp"

using namespace std;

class Game {
private:
    Board board;
    vector<Player> players;
    TurnManager turnManager;
    Dice dice;
    CardManager cardManager;
    AuctionManager auctionManager;
    BankruptcyManager bankruptcyManager;
    TradeManager tradeManager;
    LiquidationManager liquidationManager;
    Logger logger;
    Config config;
    bool gameOver;
    int lastDiceTotal;
    bool hasRolledThisTurn;
    bool extraRollPending;
    bool festivalSelectionPending;
    int pendingFestivalPlayerId;

    void giveCurrentPlayerTurnStartAbility();
    
public:
    // Constructor
    Game();
    
    // Destructor
    ~Game() = default;

    // Core Game Loop Methods
    void run();
    void startTurn();
    void endTurn();
    void handleLanding(Player& player); 
    void checkWinCondition();
    void executeCommand(const std::string& command);
    std::pair<int, int> rollDiceForCurrentPlayer();
    void endCurrentTurn();
    bool buyCurrentProperty();
    bool mortgageProperty(const std::string& code);
    bool redeemProperty(const std::string& code);
    bool buildProperty(const std::string& code);
    bool useCurrentPlayerAbilityCard(int cardIndex);
    bool useCurrentPlayerTeleportCard(int cardIndex, int tileIndex);
    bool useCurrentPlayerDemolitionCard(int cardIndex, int tileIndex);
    bool useCurrentPlayerLassoCard(int cardIndex, int targetPlayerId);
    bool discardCurrentPlayerAbilityCard(int cardIndex);
    void awardGoSalary(Player& player);
    void payBankOrBankrupt(Player& player, int amount, const std::string& reason);
    void payPlayerOrBankrupt(Player& payer, Player& receiver, int amount, const std::string& reason);
    void declareBankruptToBank(Player& player);
    void declareBankruptToPlayer(Player& player, Player& creditor);
    void setLastDiceTotal(int total);
    int getLastDiceTotal() const;
    bool getHasRolledThisTurn() const;
    bool isExtraRollPending() const;
    bool prepareExtraRollForCurrentPlayer();
    bool payJailFineForCurrentPlayer();
    bool useJailFreeCardForCurrentPlayer();
    void requestFestivalSelection(Player& player);
    bool hasPendingFestivalSelection() const;
    bool isFestivalSelectionPendingForCurrentPlayer() const;
    std::vector<std::string> buildFestivalSelectionLines(Player& player) const;
    bool applyFestivalToCurrentPlayerProperty(const std::string& code, std::vector<std::string>* messages = nullptr);

    // Getters / Setters    
    Board& getBoard();
    vector<Player>& getPlayers();
    Player& getPlayer(int index);
    TurnManager& getTurnManager();
    Dice& getDice();
    CardManager& getCardManager();
    AuctionManager& getAuctionManager();
    BankruptcyManager& getBankruptcyManager();
    TradeManager& getTradeManager();
    LiquidationManager& getLiquidationManager();
    Logger& getLogger();
    Config& getConfig();

    void setConfig(const Config& cfg);
    bool isGameOver() const;
    void setGameOver(bool status);
};
