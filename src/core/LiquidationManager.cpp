#include "../../include/core/LiquidationManager.hpp"
#include "../../include/core/Player.hpp"
#include "../../include/core/PropertyTile.hpp"
#include "../../include/core/StreetTile.hpp"
#include "../../include/core/InvalidActionException.hpp"

using namespace std;

int LiquidationManager::calculatePrice(const Player& player, const PropertyTile* property, LiquidationAction action) const {
    if (property == nullptr) return 0;

    if (action == LiquidationAction::MORTGAGE) {
        return property->getMortgageValue(); 
    } 
    else if (action == LiquidationAction::SELL_TO_BANK) {
        int price = property->getPurchasePrice();
        
        const StreetTile* street = dynamic_cast<const StreetTile*>(property);
        if (street != nullptr) {
            price += (street->getBuildingLevel() * street->getHouseBuildCost()) / 2;
        }
        return price;
    }
    return 0;
}

void LiquidationManager::liquidateAssets(Player& player, const vector<pair<PropertyTile*, LiquidationAction>>& targetProps) {
    for (const auto& item : targetProps) {
        PropertyTile* prop = item.first;
        LiquidationAction action = item.second;

        if (prop == nullptr) continue;

        if (prop->getOwner() != &player) {
            throw InvalidActionException("Gagal likuidasi: Properti " + prop->getName() + " bukan milik Anda!");
        }

        int funds = calculatePrice(player, prop, action);

        if (action == LiquidationAction::MORTGAGE) {
            StreetTile* street = dynamic_cast<StreetTile*>(prop);
            if (street != nullptr && street->getBuildingLevel() > 0) {
                throw InvalidActionException("Gagal: Harus menjual bangunan di " + prop->getName() + " terlebih dahulu sebelum digadai!");
            }
            
            prop->mortgage();
            player.credit(funds);
        } 
        
        else if (action == LiquidationAction::SELL_TO_BANK) {
            StreetTile* street = dynamic_cast<StreetTile*>(prop);
            if (street != nullptr) {
                street->sellBuildingsToBank(); 
            }
            
            player.credit(funds);
            player.removeProperty(*prop);
            
            prop->setOwner(nullptr); 
        }
    }
}

// Catatan: perubahan kelas