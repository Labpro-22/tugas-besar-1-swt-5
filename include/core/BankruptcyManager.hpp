#pragma once

class Game;
class Player;
class LiquidationManager;

class BankruptcyManager {
private:
    LiquidationManager* liquidationManager;

public:
    // Constructor & Destructor
    explicit BankruptcyManager(LiquidationManager* liqMgr = nullptr);
    ~BankruptcyManager() = default;

    // Setter
    void setLiquidationManager(LiquidationManager* liqMgr);

    bool canRecoverDebt(Player& player, int amount) const;
    int estimateMaxLiquidationValue(Player& player) const;
    void resolveDebt(Player& debtor, int amount, Game& game);
    void declareBankruptToBank(Player& player, Game& game);
    void declareBankruptToPlayer(Player& player, Player& creditor, Game& game);
    void transferAssets(Player& from, Player& to);
    void auctionBankReturnedProperties(Game& game);
};