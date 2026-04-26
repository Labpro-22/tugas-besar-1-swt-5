#include "../../include/core/GameManager.hpp"

#include "../../include/core/Game.hpp"
#include "../../include/data-layer/FileIOException.hpp"
#include "../../include/data-layer/GameStateLoader.hpp"
#include "../../include/data-layer/GameStateSaver.hpp"

GameManager::GameManager() : currentGame(nullptr) {}

GameManager::~GameManager() {
    quitCurrentGame();
}

void GameManager::startNewGame() {
    quitCurrentGame();
    currentGame = new Game();
}

void GameManager::loadGame(const string& fileName) {
    quitCurrentGame();
    currentGame = new Game();

    if (!GameStateLoader::load(*currentGame, fileName)) {
        quitCurrentGame();
        throw FileIOException("Gagal memuat game dari file: " + fileName);
    }
}

void GameManager::loadGame(const string& fileName, const AccountManager& accManager) {
    (void) accManager;
    loadGame(fileName);
}

void GameManager::saveGame(const string& fileName) const {
    if (currentGame == nullptr) {
        throw FileIOException("Gagal menyimpan game: tidak ada game aktif.");
    }

    if (!GameStateSaver::save(*currentGame, fileName)) {
        throw FileIOException("Gagal menyimpan game ke file: " + fileName);
    }
}

void GameManager::quitCurrentGame() {
    delete currentGame;
    currentGame = nullptr;
}

Game* GameManager::getCurrentGame() const {
    return currentGame;
}
