#ifndef COREDUMMY_IGAME_FACADE_HPP
#define COREDUMMY_IGAME_FACADE_HPP

#include <string>
#include <vector>
#include "GameViewModel.hpp"

class IGameFacade {
public:
    virtual ~IGameFacade() = default;

    virtual const GameViewModel& getViewModel() const = 0;
    virtual void tick(float deltaSeconds) = 0;
    virtual void startNewGame(const std::vector<std::string>& playerNames) = 0;
    virtual void loadDemoGame() = 0;
    virtual void selectTile(int index) = 0;
    virtual void rollDice() = 0;
    virtual void advanceTurn() = 0;

    virtual void openSelectedTileDetails() = 0;
    virtual void showCurrentPlayerProperties() = 0;
    virtual void showCardPopup() = 0;
    virtual void showFestivalPopup() = 0;
    virtual void showTaxPopup() = 0;
    virtual void showAuctionPopup() = 0;
    virtual void showSavePopup() = 0;
    virtual void showVictoryPopup() = 0;
    virtual void closeOverlay() = 0;
};

#endif
