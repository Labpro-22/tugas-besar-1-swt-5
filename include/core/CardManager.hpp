#pragma once
#include "../models/DrawCardDeck.hpp"
#include "../models/ChanceCard.hpp"
#include "../models/ChanceCard.hpp"
#include "../models/CommunityChestCard.hpp"
#include "../models/AbilityCardDeck.hpp"

class CardManager
{
private:
    DrawCardDeck<ChanceCard> chanceDeck;
    DrawCardDeck<CommunityChestCard> communityDeck;
    AbilityCardDeck abilityDeck;

public:
    CardManager();
    CardManager(DrawCardDeck<ChanceCard> chanceDeck,
        DrawCardDeck<CommunityChestCard> communityDeck,
        AbilityCardDeck abilityDeck);
    ChanceCard* drawChanceCard();
    CommunityChestCard* drawCommunityCard();
    std::unique_ptr<AbilityCard> drawAbilityCard();
    void giveTurnStartAbility(Player* player);
    void discardAbilityCard(std::unique_ptr<AbilityCard> card);
};
