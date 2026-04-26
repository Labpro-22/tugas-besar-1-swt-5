#pragma once
#include "AbilityCard.hpp"

class ShieldCard : public AbilityCard
{
public:
    ShieldCard();
    void use(Player* target, Game* game);
    std::string serialize() const; 
};

class TeleportCard : public AbilityCard
{
public:
    TeleportCard();
    void use(Player* target, Game* game);
    std::string serialize() const;
};

class MoveCard : public AbilityCard
{
private:
    int steps;
public:
    MoveCard();
    explicit MoveCard(int steps);
    void setup();
    void use(Player* target, Game* game);
    std::string serialize() const;
};

class LassoCard : public AbilityCard
{
public:
    LassoCard();
    void use(Player* target, Game* game);
    std::string serialize() const;
};

class DiscountCard : public AbilityCard
{
private:
    int percentage;
public:
    DiscountCard();
    explicit DiscountCard(int percentage);
    void setup();
    void use(Player* target, Game* game);
    std::string serialize() const;
};

class DemolitionCard : public AbilityCard
{
public:
    DemolitionCard();
    void use(Player* target, Game* game);
    std::string serialize() const;
};