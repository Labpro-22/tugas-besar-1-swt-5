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
public:
    TeleportCard();
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
public:
    LassoCard();
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
public:
    DemolitionCard();
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
