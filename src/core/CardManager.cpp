#include "../../include/core/CardManager.hpp"
#include "../../include/models/Player.hpp"
#include <memory>

CardManager::CardManager() {
    static StepbackCard stepbackCard;
    static RailroadCard railroadCard;
    static GoToJailCard goToJailCard;
    std::vector<ChanceCard*> chancePile({
        &stepbackCard,
        &railroadCard,
        &goToJailCard
    });
    this->chanceDeck = DrawCardDeck<ChanceCard>(chancePile);
    static BirthdayCard birthdayCard;
    static PayDoctorCard payDoctorCard;
    static NyalegCard nyalegCard;
    std::vector<CommunityChestCard*> communityPile({
        &birthdayCard,
        &payDoctorCard,
        &nyalegCard
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