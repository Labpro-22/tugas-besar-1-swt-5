#include "utils/PropertyTile.hpp"
#include "utils/RailroadTile.hpp"
#include "utils/StreetTile.hpp"
#include "utils/UtilityTile.hpp"
#include "core/Game.hpp"
#include "models/Player.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>

PropertyTile::PropertyTile(int id, std::string code, std::string name, int mortgageValue)
: Tile(id, code, name),
      owner(nullptr),
      status(BANK),
      mortgageValue(mortgageValue),
      festivalMultiplier(1),
      festivalDuration(0) {}

void PropertyTile::onLand(Player* player, Game* game) {
    if (player == nullptr || game == nullptr) {
        return;
    }

    if (status == MORTGAGED) {
        std::cout << name << " sedang digadaikan. Tidak ada sewa.\n";
        return;
    }

    if (status == BANK) {
        if (dynamic_cast<RailroadTile*>(this) != nullptr || dynamic_cast<UtilityTile*>(this) != nullptr) {
            setOwner(player);
            auto& owned = player->getOwnedProperties();
            if (std::find(owned.begin(), owned.end(), this) == owned.end()) {
                player->addProperty(this);
            }
            game->getLogger().log(game->getTurnManager().getCurrentTurn(),
                                  player->getUsername(), "AKUISISI",
                                  name + " otomatis dari bank");
            std::cout << player->getUsername() << " mendapatkan " << name << " otomatis.\n";
            return;
        }

        if (dynamic_cast<StreetTile*>(this) != nullptr) {
            game->getLogger().log(game->getTurnManager().getCurrentTurn(),
                                  player->getUsername(), "MENUNGGU_BELI",
                                  name + " tersedia M" + std::to_string(getLandPrice()));
            std::cout << name << " tersedia. Pilih BELI atau LELANG.\n";
        }
        return;
    }

    if (status == OWNED && owner != nullptr && owner != player) {
        const int rent = calculateRent(player, game);
        std::cout << player->getUsername() << " membayar sewa " << name
                  << " M" << rent << " kepada " << owner->getUsername() << ".\n";
        game->payPlayerOrBankrupt(*player, *owner, rent, "Sewa " + name);
    }
}

void PropertyTile::setOwner(Player* owner) {
    this->owner = owner;
    status = (owner == nullptr ? BANK : OWNED);
}

Player* PropertyTile::getOwner() const { return owner; }
PropertyStatus PropertyTile::getStatus() const { return status; }
int PropertyTile::getMortgageValue() const { return mortgageValue; }

int PropertyTile::mortgage() {
    status = MORTGAGED;
    return mortgageValue;
}

int PropertyTile::redeem() {
    status = OWNED;
    return getLandPrice();
}

bool PropertyTile::isOwned() const { return status == OWNED; }
bool PropertyTile::isMortgaged() const { return status == MORTGAGED; }

void PropertyTile::applyFestival() {
    if (festivalMultiplier < 8)
        festivalMultiplier *= 2;
    festivalDuration = 3;
}

void PropertyTile::decrementFestivalDuration() {
    if (festivalDuration > 0)
        festivalDuration--;
}

void PropertyTile::resetFestivalIfExpired() {
    if (festivalDuration == 0)
        festivalMultiplier = 1;
}

int PropertyTile::getFestivalMultiplier() const { return festivalMultiplier; }
int PropertyTile::getFestivalDuration() const { return festivalDuration; }

std::string PropertyTile::toString() const {
    std::ostringstream oss;
    oss << name << " (" << code << ")"
        << " | status=" << (status == BANK ? "BANK" : status == OWNED ? "OWNED" : "MORTGAGED")
        << " | festival=x" << festivalMultiplier
        << " dur=" << festivalDuration;
    return oss.str();
}

bool PropertyTile::canBeMortgaged(Game* game) const {
    (void) game;
    return status == OWNED;
}

bool PropertyTile::isOwnable() const { return true; }

void PropertyTile::cetakAkta() const {
    std::cout << "+================================+" << std::endl;
    std::cout << "|        AKTA KEPEMILIKAN        |" << std::endl;
    std::string info = "[DEFAULT] " + name + " (" + code + ")";
    int left = (33 - info.length()) / 2;
    int right = 33 - left;
    std::cout << "|";
    for (size_t i = 0; i < left; i++){std::cout << " ";}
    std::cout << info;
    for (size_t i = 0; i < right; i++){std::cout << " ";}
    std::cout << "|" << std::endl;
    std::cout << "+================================+" << std::endl;
    printf("| Harga Beli        : M%3d       |\n", getLandPrice());
    printf("| Harga Gadai       : M%3d       |\n", getMortgageValue());
    printf("+================================+\n");
    std::string statusString;
    switch (status)
    {
    case PropertyStatus::MORTGAGED:
        statusString = "MORTGAGED (" + this->getOwner()->getUsername() + ")";
        break;
    case PropertyStatus::BANK:
        statusString = "BANK";
        break;
    case PropertyStatus::OWNED:
        statusString = "OWNED (" + this->getOwner()->getUsername() + ")";
        break;   
    }
    printf("| Status : ");
    printf(statusString.c_str());
    for (int i = 0; i < 20 - statusString.length(); i++) {printf(" ");}
    printf(" |\n");
    printf("+================================+\n");
}