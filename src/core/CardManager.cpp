#include "../../include/core/CardManager.hpp"
#include "../../include/models/Player.hpp"
#include <memory>

CardManager::CardManager() {
    std::vector<ChanceCard> chancePile({
        StepbackCard(),
        RailroadCard(),
        GoToJailCard()
    });
    this->chanceDeck = DrawCardDeck<ChanceCard>(chancePile);
    std::vector<CommunityChestCard> communityPile({
        BirthdayCard(),
        PayDoctorCard(),
        NyalegCard()
    });
    this->communityDeck = DrawCardDeck<CommunityChestCard>(communityPile);    
}

CardManager::CardManager(DrawCardDeck<ChanceCard> chanceDeck,
    DrawCardDeck<CommunityChestCard> communityDeck,
    AbilityCardDeck abilityDeck) : chanceDeck(chanceDeck),
    communityDeck(communityDeck), abilityDeck(abilityDeck) {}

ChanceCard* CardManager::drawChanceCard() {
    return this->chanceDeck.draw();
}
CommunityChestCard* CardManager::drawCommunityCard() {
    return this->communityDeck.draw();
}
std::unique_ptr<AbilityCard> CardManager::drawAbilityCard() {
    return this->abilityDeck.draw();
}
void CardManager::giveTurnStartAbility(Player* player) {
    player->addCard(this->abilityDeck.draw());
}
void CardManager::discardAbilityCard(std::unique_ptr<AbilityCard> card) {
    this->abilityDeck.discard(std::move(card));
}