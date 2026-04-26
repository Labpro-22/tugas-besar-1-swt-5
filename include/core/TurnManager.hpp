#pragma once

#include <vector>

using namespace std;

class TurnManager {
private:
    vector<int> turnOrder;
    int currentPlayerIndex;     
    int currentTurn;           
    int maxTurn;                    

public:
    // Constructor & Destructor
    TurnManager() = default;
    TurnManager(const vector<int>& initialTurnOrder, int maxTurnLimit);
    ~TurnManager() = default;

    // Core Logic Methods
    void nextPlayer();
    void grantExtraTurn();
    void removePlayer(int playerId);
    bool isMaxTurnReached() const;

    // Getters    
    int getCurrentPlayerIndex() const;
    int getCurrentTurn() const;
    int getMaxTurn() const;

    //Setters
    void setCurrentPlayerIndex(int CurrIdx);
    void setCurrentTurn(int CurrentTurn);
};