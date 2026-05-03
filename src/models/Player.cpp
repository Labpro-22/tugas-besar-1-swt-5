#include "../../include/models/Player.hpp"
#include "../../include/core/Account.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/core/InsufficientFundException.hpp"
#include "../../include/core/BankruptException.hpp"
#include <stdexcept>

void Player::moveTo(int position) {
    this->position = position;
}
void Player::pay(int amount) {
    if (amount > this->money) {
        if (this->ownedProperties.empty()) {
            throw BankruptException(1, "gagal bayar, ga punya properti juga");
        }
        throw InsufficientFundException(amount, this->money);
    }
    this->money -= amount;
}
void Player::receive(int amount) {
    this->money += amount;
}
bool Player::canAfford(int amount) const {
    return this->money >= amount;
}
void Player::addProperty(PropertyTile* prop) {
    this->ownedProperties.push_back(prop);
}
void Player::removeProperty(PropertyTile* prop) {
    this->ownedProperties.erase(
        std::remove(this->ownedProperties.begin(), this->ownedProperties.end(), prop),
        this->ownedProperties.end()
    );
}
void Player::addCard(std::unique_ptr<AbilityCard> card) {
    this->handCards.push_back(std::move(card));
}
std::unique_ptr<AbilityCard> Player::dropCard(int index) {
    if (index < 0 || index >= this->handCards.size()) {
        throw std::invalid_argument("Index out of bound");
    }
    auto card = std::move(this->handCards[index]);
    this->handCards.erase(this->handCards.begin() + index);
    return card;
}
void Player::enterJail() {
    this->status = PlayerStatus::JAILED;
    this->jailTurnsAttempted = 0;
    this->consecutiveDoubleCount = 0;
}
void Player::releaseFromJail() {
    this->status = PlayerStatus::ACTIVE;
    this->jailTurnsAttempted = 0;
    this->consecutiveDoubleCount = 0;
}
void Player::incrementJailAttempt() {
    this->jailTurnsAttempted++;
}
void Player::resetTurnFlags() {
    this->usedAbilityThisTurn = false;
    this->shieldActive = false;
    if (this->discountDuration > 0) {
        this->discountDuration--;
        if (this->discountDuration == 0) {
            this->discountPercent = 0;
        }
    }
    // consecutiveDoubleCount dan jailTurnsAttempted dikelola oleh Game/TurnManager
}
int Player::getTotalWealth() const {
    int wealth = money;
    for (PropertyTile* tile : this->ownedProperties) {
        // Semua properti dihitung dengan landPrice penuh (termasuk MORTGAGED)
        wealth += tile->getLandPrice();
        // Hitung nilai bangunan jika StreetTile
        const StreetTile* street = dynamic_cast<const StreetTile*>(tile);
        if (street != nullptr) {
            wealth += street->getHouseCount() * street->getHouseBuildCost();
            if (street->hasHotelBuilt()) {
                wealth += street->getHotelBuildCost();
            }
        }
    }
    return wealth;
}
bool Player::isBankrupt() const {
    return this->status == PlayerStatus::BANKRUPT;
}
std::string Player::toString() const {
    return getUsername() + " " + std::to_string(this->money);
}
void Player::useAbilityCard(int index, Game* game) {
    this->handCards.at(index)->use(this, game);
}
bool Player::operator<(const Player& other) const {
    return this->getTotalWealth() < other.getTotalWealth();
}
bool Player::operator>(const Player& other) const {
    return this->getTotalWealth() > other.getTotalWealth();
}
bool Player::operator==(const Player& other) const {
    return this->getTotalWealth() == other.getTotalWealth();
}
std::vector<PropertyTile*>& Player::getOwnedProperties() {
    return this->ownedProperties;
}

std::string Player::getUsername() const {
    if (this->account == nullptr) {
        return "COM" + std::to_string(this->id);
    }
    return this->account->getUsername();
}

int Player::getId() const {
    return id;
}

int Player::getPosition() const {
    return this->position;
}


void Player::activateShield() {
    this->shieldActive = true;
}

void Player::applyDiscount(int percentage) {
    if (percentage < 0) {
        percentage = 0;
    }

    if (percentage > 100) {
        percentage = 100;
    }

    this->discountPercent = percentage;
    this->discountDuration = 1;
}
