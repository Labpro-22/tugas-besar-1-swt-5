#pragma once
#include "AbilityCard.hpp"

class ShieldCard : public AbilityCard
{
public:
    ShieldCard();
    void use(Player* target, Game* game) override;
    std::string serialize() const override;
};

class TeleportCard : public AbilityCard
{
private:
    int destination;

public:
    TeleportCard();
    void setDestination(int tileIndex);
    void use(Player* target, Game* game) override;
    std::string serialize() const override;
};

class MoveCard : public AbilityCard
{
private:
    int steps;

public:
    MoveCard();
    explicit MoveCard(int steps);

    void setup() override;
    void use(Player* target, Game* game) override;
    std::string serialize() const override;
};

class LassoCard : public AbilityCard
{
private:
    int targetPlayerId;

public:
    LassoCard();
    void setTargetPlayerId(int playerId);
    void use(Player* target, Game* game) override;
    std::string serialize() const override;
};

class DiscountCard : public AbilityCard
{
private:
    int percentage;

public:
    DiscountCard();
    explicit DiscountCard(int percentage);

    void setup() override;
    void use(Player* target, Game* game) override;
    std::string serialize() const override;
};

class DemolitionCard : public AbilityCard
{
private:
    int targetTileIndex;

public:
    DemolitionCard();
    void setTargetTileIndex(int tileIndex);
    void use(Player* target, Game* game) override;
    std::string serialize() const override;
};


class JailFreeCard : public AbilityCard
{
public:
    JailFreeCard();
    void use(Player* target, Game* game) override;
    std::string serialize() const override;
};
