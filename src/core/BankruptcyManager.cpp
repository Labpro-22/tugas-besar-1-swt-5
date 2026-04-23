#include "../../include/core/BankruptcyManager.hpp"
#include "../../include/core/LiquidationManager.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/core/PropertyTile.hpp"
#include "../../include/core/StreetTile.hpp"
#include "../../include/core/Game.hpp"
#include "../../include/core/AuctionManager.hpp"
#include "../../include/core/TurnManager.hpp"
#include "../../include/core/BankruptException.hpp"
#include "../../include/core/Logger.hpp"

#include <vector>

using namespace std;

BankruptcyManager::BankruptcyManager(LiquidationManager* liqMgr) : liquidationManager(liqMgr) {}

void BankruptcyManager::setLiquidationManager(LiquidationManager* liqMgr) {
    this->liquidationManager = liqMgr;
}

int BankruptcyManager::estimateMaxLiquidationValue(const Player& player) const {
    int totalValue = player.getMoney();

    for (const PropertyTile* prop : player.getOwnedProperties()) {
        totalValue += prop->getPurchasePrice();
        
        const StreetTile* street = dynamic_cast<const StreetTile*>(prop);
        if (street != nullptr) {
            totalValue += (street->getBuildingLevel() * street->getHouseBuildCost()) / 2;
        }
    }
    return totalValue;
}

bool BankruptcyManager::canRecoverDebt(const Player& player, int amount) const {
    return estimateMaxLiquidationValue(player) >= amount;
}

void BankruptcyManager::resolveDebt(Player& debtor, int amount, Game& game) {
    if (!canRecoverDebt(debtor, amount)) {
        throw BankruptException(1, debtor.getUsername() + " tidak memiliki aset yang cukup dan dinyatakan BANGKRUT!");
    }
}

// Bangkrut ke Pemain
void BankruptcyManager::declareBankruptToPlayer(Player& bankruptPlayer, Player& creditor, Game& game) {
    game.getLogger().log(game.getTurnManager().getCurrentTurn(), 
                         bankruptPlayer.getUsername(), 
                         "BANGKRUT", 
                         "Aset diserahkan ke " + creditor.getUsername());

    transferAssets(bankruptPlayer, creditor);
    // game.getTurnManager().removePlayer(bankruptPlayer.getId()); 
}

void BankruptcyManager::transferAssets(Player& from, Player& to) {
    to.credit(from.getMoney());
    from.debit(from.getMoney());

    vector<PropertyTile*> propertiesToTransfer = from.getOwnedProperties();
    for (PropertyTile* prop : propertiesToTransfer) {
        prop->setOwner(to);
        to.addProperty(*prop);
        from.removeProperty(*prop);
    }
}

// Bangkrut ke Bank 
void BankruptcyManager::declareBankruptToBank(Player& bankruptPlayer, Game& game) {
    game.getLogger().log(game.getTurnManager().getCurrentTurn(), 
                         bankruptPlayer.getUsername(), 
                         "BANGKRUT", 
                         "Disita oleh Bank dan dilelang");

    bankruptPlayer.debit(bankruptPlayer.getMoney());

    vector<PropertyTile*> propertiesToAuction = bankruptPlayer.getOwnedProperties();
    for (PropertyTile* prop : propertiesToAuction) {
        StreetTile* street = dynamic_cast<StreetTile*>(prop);
        if (street != nullptr) {
            street->sellBuildingsToBank(); 
        }
        
        prop->setOwner(nullptr);
        bankruptPlayer.removeProperty(*prop);
    }

    // game.getTurnManager().removePlayer(bankruptPlayer.getId());

    auctionBankReturnedProperties(game);
}

// Lelang Aset Sitaan
void BankruptcyManager::auctionBankReturnedProperties(Game& game) {
    // CATATAN
    // Contoh: game.queuePropertiesForAuction(propertiesToAuction);
}

// Catatan: Perhatikan catatan lain, ini belum selesai, perhatikan GUI