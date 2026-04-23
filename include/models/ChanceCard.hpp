#pragma once
#include "DrawCard.hpp"

class ChanceCard : public DrawCard
{
public:
    ChanceCard(std::string description, std::function<void(Player*, Game*)> effect)
        : DrawCard("Kartu Kesempatan\n" + description, effect) {}
    virtual ~ChanceCard() = default;
};

class StepbackCard : public ChanceCard
{
public:
    StepbackCard();
};

class RailroadCard : public ChanceCard
{
public:
    RailroadCard();
};

class GoToJailCard : public ChanceCard
{
public:
    GoToJailCard();
};