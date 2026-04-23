#pragma once
#include "AbilityCard.hpp"

class ShieldCard : public AbilityCard
{
public:
    ShieldCard();
    void use(Player* target, Game* game);
};

class TeleportCard : public AbilityCard
{
public:
    TeleportCard();
    void use(Player* target, Game* game);
};

class MoveCard : public AbilityCard
{
private:
    int steps;
public:
    MoveCard();
    void setup();
    void use(Player* target, Game* game);
};

class LassoCard : public AbilityCard
{
public:
    LassoCard();
    void use(Player* target, Game* game);
};

class DiscountCard : public AbilityCard
{
private:
    int percentage;
public:
    DiscountCard();
    void setup();
    void use(Player* target, Game* game);
};

class DemolitionCard : public AbilityCard
{
public:
    DemolitionCard();
    void use(Player* target, Game* game);
};