#pragma once

#include <vector>
#include <string>

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
    void giveSalary(Player& player);
    void checkWinCondition();
    void executeCommand(const std::string& command);

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