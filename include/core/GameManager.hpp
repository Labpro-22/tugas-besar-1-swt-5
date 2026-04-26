#pragma once

#include <string>

using namespace std;

class Game;
class AccountManager;

class GameManager {
private:
    Game* currentGame;

public:
    // Constructor
    GameManager();
    
    // Destructor
    ~GameManager(); 

    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    // Core Functionalities
    void startNewGame();
    void loadGame(const string& fileName);
    void saveGame(const string& fileName) const;
    
    // Helper
    void quitCurrentGame(); 

    // Getters
    Game* getCurrentGame() const;
};