#pragma once
#include "DrawCard.hpp"

class CommunityChestCard : public DrawCard
{
public:
    CommunityChestCard(std::string description, std::function<void(Player*, Game*)> effect)
        : DrawCard("Kartu Dana Umum\n" + description, effect) {}
    virtual ~CommunityChestCard() = default;
};

class BirthdayCard : public CommunityChestCard
{
public:
    BirthdayCard();
};

class PayDoctorCard : public CommunityChestCard
{
public:
    PayDoctorCard();
};

class NyalegCard : public CommunityChestCard
{
public:
    NyalegCard();
};

