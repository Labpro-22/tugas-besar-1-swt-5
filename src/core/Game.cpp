#include "../../include/core/Game.hpp"

Game::Game()
    : board(),
      players(),
      turnManager(std::vector<int>{}, 0),
      dice(),
      cardManager(),
      auctionManager(),
      bankruptcyManager(&liquidationManager),
      tradeManager(),
      liquidationManager(),
      logger(),
      config(),
      gameOver(false) {}

void Game::run() {}
void Game::startTurn() {}
void Game::endTurn() {}

void Game::handleLanding(Player& player) {
    Tile* tile = board.getTileByIndex(player.getPosition());
    if (tile != nullptr) {
        tile->onLand(&player, this);
    }
}

void Game::checkWinCondition() {}
void Game::executeCommand(const std::string&) {}

Board& Game::getBoard() { return board; }
vector<Player>& Game::getPlayers() { return players; }
Player& Game::getPlayer(int index) { return players.at(static_cast<std::size_t>(index)); }
TurnManager& Game::getTurnManager() { return turnManager; }
Dice& Game::getDice() { return dice; }
CardManager& Game::getCardManager() { return cardManager; }
AuctionManager& Game::getAuctionManager() { return auctionManager; }
BankruptcyManager& Game::getBankruptcyManager() { return bankruptcyManager; }
TradeManager& Game::getTradeManager() { return tradeManager; }
LiquidationManager& Game::getLiquidationManager() { return liquidationManager; }
Logger& Game::getLogger() { return logger; }
Config& Game::getConfig() { return config; }

bool Game::isGameOver() const { return gameOver; }
void Game::setGameOver(bool status) { gameOver = status; }
