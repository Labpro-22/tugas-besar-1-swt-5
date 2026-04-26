#ifndef REAL_GAME_FACADE_HPP
#define REAL_GAME_FACADE_HPP

#include <string>
#include <vector>
#include "../coredummy/IGameFacade.hpp"
#include "../coredummy/GameViewModel.hpp"
#include "../core/GameManager.hpp"
#include "../core/AccountManager.hpp"
#include "../data-layer/AccountDataManager.hpp"
#include "../data-layer/ConfigComposer.hpp"

// RealGameFacade mengimplementasi IGameFacade yang sama dengan MockGameFacade,
// sehingga UI Layer tidak perlu diubah sama sekali — hanya swap facade-nya.
class RealGameFacade : public IGameFacade {
public:
    RealGameFacade();
    ~RealGameFacade() override = default;

    const GameViewModel& getViewModel() const override;
    void tick(float deltaSeconds) override;
    void startNewGame(const std::vector<std::string>& playerNames) override;
    void loadDemoGame() override;
    void selectTile(int index) override;
    void rollDice() override;
    void advanceTurn() override;

    void openSelectedTileDetails() override;
    void showCurrentPlayerProperties() override;
    void showCardPopup() override;
    void showFestivalPopup() override;
    void showTaxPopup() override;
    void showAuctionPopup() override;
    void showSavePopup() override;
    void showVictoryPopup() override;
    void closeOverlay() override;

    // Akses langsung ke GameManager dan AccountManager untuk InGameScene
    GameManager& getGameManager() { return gameManager; }
    AccountManager& getAccountManager() { return accountManager; }

private:
    GameManager gameManager;
    AccountManager accountManager;
    GameViewModel vm;

    void rebuildViewModel();
    void showOverlay(OverlayType type, const std::string& title,
                     const std::vector<std::string>& lines,
                     const std::string& footer);
    TileKind tileKindFromCode(const std::string& code, const std::string& colorGroup) const;
};

#endif
