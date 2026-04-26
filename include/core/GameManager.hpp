#pragma once

#include <string>

using namespace std;

class Game;
class AccountManager;

class GameManager {
private:
    Game* currentGame;

public:
    GameManager();
    ~GameManager();

    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    void startNewGame();
    void loadGame(const string& fileName);
    void loadGame(const string& fileName, const AccountManager& accManager);
    void saveGame(const string& fileName) const;

    void quitCurrentGame();

    Game* getCurrentGame() const;
};
