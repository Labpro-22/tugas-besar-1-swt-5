#include "../../include/models/Player.hpp"
#include "../../include/models/Account.hpp"
#include "../../include/models/AbilityCard.hpp"
#include <stdexcept>

void Player::moveTo(int position) {
    this->position = position;
}
void Player::pay(int amount) {
    if (amount > this->money) {
        // TODO: THROW EXCEPTION
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
    return this->ownedProperties.push_back(prop);
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
}
void Player::releaseFromJail() {
    this->status = PlayerStatus::ACTIVE;
}
void Player::incrementJailAttempt() {
    this->jailTurnsAttempted++;
}
void Player::resetTurnFlags() {
    this->jailTurnsAttempted = 0;
    this->usedAbilityThisTurn = false;
    this->consecutiveDoubleCount = 0;
}
int Player::getTotalWealth() const {
    return money;
    // TODO: CALCULATE WEALTH FROM PROPERTY
}
bool Player::isBankrupt() const {
    return this->status == PlayerStatus::BANKRUPT;
}
std::string Player::toString() const {
    // TODO: FORMAT STRING
    // <USERNAME> <UANG> <KODE_PETAK> <STATUS>
    // <JUMLAH_KARTU_TANGAN>
    // <JENIS_KARTU_1> <NILAI_KARTU_1> <SISA_DURASI_1>
    // <JENIS_KARTU_2> <NILAI_KARTU_2> <SISA_DURASI_2>

    return this->account->getName() + " " + std::to_string(this->money);
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