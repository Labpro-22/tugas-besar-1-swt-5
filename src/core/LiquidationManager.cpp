#include "../../include/core/LiquidationManager.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/core/InvalidActionException.hpp"

using namespace std;

namespace {
int calculateBuildingSaleValue(const StreetTile* street) {
    if (street == nullptr) {
        return 0;
    }

    int total = street->getHouseCount() * street->getHouseBuildCost();
    if (street->hasHotelBuilt()) {
        total += street->getHotelBuildCost();
    }

    return total / 2;
}

int sellColorGroupBuildingsToBank(Player& player, StreetTile& selected) {
    int total = 0;

    for (PropertyTile* owned : player.getOwnedProperties()) {
        StreetTile* street = dynamic_cast<StreetTile*>(owned);
        if (street == nullptr || street->getColorGroup() != selected.getColorGroup()) {
            continue;
        }

        total += street->sellBuildingsToBank();
    }

    if (total > 0) {
        player.receive(total);
    }

    return total;
}
}

int LiquidationManager::calculatePrice(const Player& player, const PropertyTile* property, LiquidationAction action) const {
    (void) player;
    if (property == nullptr) return 0;

    if (action == LiquidationAction::MORTGAGE) {
        if (property->isMortgaged()) {
            return 0;
        }
        return property->getMortgageValue(); 
    } 
    else if (action == LiquidationAction::SELL_TO_BANK) {
        if (property->isMortgaged()) {
            return 0;
        }

        int price = property->getLandPrice();
        
        const StreetTile* street = dynamic_cast<const StreetTile*>(property);
        if (street != nullptr) {
            price += calculateBuildingSaleValue(street);
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

        if (action == LiquidationAction::MORTGAGE) {
            if (prop->getStatus() != OWNED) {
                throw InvalidActionException("Gagal: Properti " + prop->getName() + " tidak dapat digadaikan!");
            }

            StreetTile* street = dynamic_cast<StreetTile*>(prop);
            if (street != nullptr) {
                sellColorGroupBuildingsToBank(player, *street);
            }
            
            int funds = calculatePrice(player, prop, action);
            prop->mortgage();
            player.receive(funds);
        } 
        
        else if (action == LiquidationAction::SELL_TO_BANK) {
            if (prop->isMortgaged()) {
                throw InvalidActionException("Gagal: Properti " + prop->getName() + " sedang digadaikan. Tebus terlebih dahulu sebelum dijual ke Bank!");
            }

            int funds = calculatePrice(player, prop, action);
            StreetTile* street = dynamic_cast<StreetTile*>(prop);
            if (street != nullptr) {
                street->sellBuildingsToBank(); 
            }
            
            player.receive(funds);
            player.removeProperty(prop);
            
            prop->setOwner(nullptr); 
        }
    }
}

// Catatan: perubahan kelas
