#include "../../include/core/GameManager.hpp"

#include <fstream>
#include "../../include/core/Game.hpp"
#include "../../include/data-layer/FileIOException.hpp"

GameManager::GameManager() : currentGame(nullptr) {}

GameManager::~GameManager() {
    quitCurrentGame();
}

void GameManager::startNewGame() {
    quitCurrentGame();
    currentGame = new Game();
}

void GameManager::loadGame(const string& fileName) {
    ifstream input(fileName);
    if (!input.is_open()) {
        throw FileIOException("Gagal memuat game: file tidak ditemukan atau tidak dapat dibuka: " + fileName);
    }

    quitCurrentGame();
    currentGame = new Game();
    currentGame->getLogger().log(0, "System", "LOAD", "Load placeholder dari " + fileName);
}

void GameManager::saveGame(const string& fileName) const {
    if (currentGame == nullptr) {
        throw FileIOException("Gagal menyimpan game: tidak ada game aktif.");
    }

    ofstream output(fileName);
    if (!output.is_open()) {
        throw FileIOException("Gagal menyimpan game: file tidak dapat ditulis: " + fileName);
    }

    output << "NIMONSPOLI_SAVE_PLACEHOLDER\n";
    output << "turn " << currentGame->getTurnManager().getCurrentTurn() << "\n";
    output << "players " << currentGame->getPlayers().size() << "\n";
    output << "game_over " << (currentGame->isGameOver() ? 1 : 0) << "\n";
}

void GameManager::quitCurrentGame() {
    delete currentGame;
    currentGame = nullptr;
}

Game* GameManager::getCurrentGame() const {
    return currentGame;
}