#ifndef COREDUMMY_MOCK_GAME_FACADE_HPP
#define COREDUMMY_MOCK_GAME_FACADE_HPP

#include <string>
#include <vector>
#include "IGameFacade.hpp"

class MockGameFacade : public IGameFacade {
public:
    MockGameFacade();

    const GameViewModel& getViewModel() const override;
    void tick(float deltaSeconds) override;
    std::string validateNewGameSettings(const std::vector<std::string>& playerNames,
                                        const std::string& configDirectory) const override;
    bool startNewGame(const std::vector<std::string>& playerNames,
                      const std::string& configDirectory) override;
    void loadDemoGame() override;
    void selectTile(int index) override;
    void rollDice() override;
    void advanceTurn() override;
    void buyCurrentProperty() override;
    void mortgageSelectedProperty() override;
    void redeemSelectedProperty() override;
    void buildSelectedProperty() override;

    void openSelectedTileDetails() override;
    void showCurrentPlayerProperties() override;
    void showCardPopup() override;
    void showFestivalPopup() override;
    void showTaxPopup() override;
    void showAuctionPopup() override;
    void showSavePopup() override;
    void showVictoryPopup() override;
    void closeOverlay() override;

private:
    GameViewModel vm;
    float elapsedSeconds = 0.0f;
    int rollCounter = 0;
    int salary = 200;

    void loadDummyBoard();
    void addLog(const std::string& actor, const std::string& type, const std::string& detail);
    void showOverlay(OverlayType type, const std::string& title, const std::vector<std::string>& lines, const std::string& footer);
    void normalizeCurrentPlayer();
    void applyDummyLandingEffect(PlayerViewData& player, int previousPosition, int diceTotal);
    std::string tileKindToText(TileKind kind) const;
    std::string propertyStatusToText(PropertyStatusView status) const;
};

#endif
