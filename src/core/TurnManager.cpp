#include "../../include/core/TurnManager.hpp"
#include <algorithm>

using namespace std;

TurnManager::TurnManager(const vector<int>& initialTurnOrder, int maxTurnLimit)
    : turnOrder(initialTurnOrder), currentPlayerIndex(0), currentTurn(1), maxTurn(maxTurnLimit) {}


void TurnManager::nextPlayer() {
    if (turnOrder.empty()) return;
    currentPlayerIndex++;
    if (currentPlayerIndex >= turnOrder.size()) {
        currentPlayerIndex = 0;
        currentTurn++;
    }
}

void TurnManager::grantExtraTurn() {
    if (turnOrder.empty()) return;
    if (currentPlayerIndex == 0) {
        currentPlayerIndex = turnOrder.size() - 1;
    } else {
        currentPlayerIndex--;
    }
}

void TurnManager::removePlayer(int playerId) {
    for (auto it = turnOrder.begin(); it != turnOrder.end(); ) {
        if (*it == playerId) {
            int removedIndex = distance(turnOrder.begin(), it);
            it = turnOrder.erase(it);
            if (removedIndex < currentPlayerIndex) {
                currentPlayerIndex--;
            } 
            else if (removedIndex == currentPlayerIndex) {
                if (currentPlayerIndex >= turnOrder.size()) {
                    currentPlayerIndex = 0;
                    currentTurn++;
                }
            }
            break;
        } else {
            ++it;
        }
    }
}

bool TurnManager::isMaxTurnReached() const {
    if (maxTurn <= 0) {
        return false;
    }
    return currentTurn >= maxTurn;
}

int TurnManager::getCurrentPlayerIndex() const {
    if (turnOrder.empty()) return -1;
    return turnOrder[currentPlayerIndex];
}

int TurnManager::getCurrentTurn() const {
    return currentTurn;
}

int TurnManager::getMaxTurn() const {
    return maxTurn;
}

void TurnManager::setCurrentPlayerIndex(int currIdx) {
    if (turnOrder.empty()) {
        currentPlayerIndex = 0;
        return;
    }

    if (currIdx < 0 || currIdx >= static_cast<int>(turnOrder.size())) {
        currentPlayerIndex = 0;
        return;
    }

    currentPlayerIndex = currIdx;
}

void TurnManager::setCurrentTurn(int currTurn) {
    this->currentTurn = currTurn;
}

const vector<int>& TurnManager::getTurnOrder() const {
    return turnOrder;
}

int TurnManager::getCurrentPlayerOrderIndex() const {
    return currentPlayerIndex;
}

void TurnManager::setMaxTurn(int maxTurn) {
    this->maxTurn = maxTurn;
}

void TurnManager::setTurnOrder(const vector<int>& turnOrder) {
    this->turnOrder = turnOrder;

    if (this->turnOrder.empty()) {
        this->currentPlayerIndex = 0;
    } else if (this->currentPlayerIndex < 0 ||
               this->currentPlayerIndex >= static_cast<int>(this->turnOrder.size())) {
        this->currentPlayerIndex = 0;
    }
}

bool TurnManager::isLastPlayerInCurrentTurn() const {
    if (turnOrder.empty()) {
        return true;
    }

    return currentPlayerIndex >= static_cast<int>(turnOrder.size()) - 1;
}