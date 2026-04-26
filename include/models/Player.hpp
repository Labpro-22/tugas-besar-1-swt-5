#pragma once
#include <vector>
#include <memory>
#include <string>
#include <utility>
#include <algorithm>
#include "AbilityCard.hpp"

class Game;
class AuctionManager;
class PropertyTile;
class Account;

enum class PlayerStatus {
    ACTIVE,
    JAILED,
    BANKRUPT
};

class Player
{
private:
    int id;
    Account* account;
    int money;
    int position;
    PlayerStatus status;
    std::vector<PropertyTile*> ownedProperties;
    std::vector<std::unique_ptr<AbilityCard>> handCards;
    int jailTurnsAttempted;
    bool usedAbilityThisTurn;
    bool shieldActive;
    int discountPercent;
    int discountDuration;
    int consecutiveDoubleCount;

public:
    friend class Game;
    Player(int id, Account* account, int money) : id(id), account(account), money(money), position(0), status(PlayerStatus::ACTIVE),
        ownedProperties(), handCards(), jailTurnsAttempted(0), usedAbilityThisTurn(false),
        shieldActive(false), discountPercent(0), discountDuration(0), consecutiveDoubleCount(0) {}
    Player(int id, Account* account, int money, int position, PlayerStatus status, std::vector<PropertyTile*> ownedProperties,
        std::vector<std::unique_ptr<AbilityCard>> handCards, int jailTurnsAttempted, bool usedAbilityThisTurn, bool shieldActive,
        int discountPercent, int discountDuration, int consecutiveDoubleCount)
            : id(id), account(account), money(money),
            position(position), status(status), ownedProperties(std::move(ownedProperties)), handCards(std::move(handCards)),
            jailTurnsAttempted(jailTurnsAttempted), usedAbilityThisTurn(usedAbilityThisTurn), shieldActive(shieldActive),
            discountPercent(discountPercent), discountDuration(discountDuration), consecutiveDoubleCount(consecutiveDoubleCount) {}
    void moveTo(int position);
    void pay(int amount);
    void receive(int amount);
    bool canAfford(int amount) const;
    void addProperty(PropertyTile* prop);
    void removeProperty(PropertyTile* prop);
    void addCard(std::unique_ptr<AbilityCard> card);
    std::unique_ptr<AbilityCard> dropCard(int index);
    void useAbilityCard(int index, Game* game);
    void enterJail();
    void releaseFromJail();
    void incrementJailAttempt();
    void resetTurnFlags();
    int getTotalWealth() const;
    bool isBankrupt() const;
    std::string toString() const;
    bool operator<(const Player& other) const;
    bool operator>(const Player& other) const;
    bool operator==(const Player& other) const;
    std::vector<PropertyTile*>& getOwnedProperties();
    std::string getUsername() const;
    int getId() const;
    int getPosition() const;
    int getMoney() const { return money; }
    PlayerStatus getStatus() const { return status; }
    int getJailTurnsAttempted() const { return jailTurnsAttempted; }
    bool hasUsedAbilityThisTurn() const { return usedAbilityThisTurn; }
    bool isShieldActive() const { return shieldActive; }
    int getDiscountPercent() const { return discountPercent; }
    int getDiscountDuration() const { return discountDuration; }
    int getConsecutiveDoubleCount() const { return consecutiveDoubleCount; }
    const std::vector<std::unique_ptr<AbilityCard>>& getHandCards() const { return handCards; }
    void setStatus(PlayerStatus s) { status = s; }
    void addConsecutiveDouble() { consecutiveDoubleCount++; }
    void resetConsecutiveDouble() { consecutiveDoubleCount = 0; }
    void cetakProperti() const;
};
