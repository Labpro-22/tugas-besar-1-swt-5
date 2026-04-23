#include "../../include/models/AbilityCardDeck.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/models/DerivedAbilityCard.hpp"
#include <algorithm>
#include <random>

AbilityCardDeck::AbilityCardDeck() {
    for (size_t i = 0; i < 4; i++)
    {
        drawPile.push_back(std::make_unique<MoveCard>());
    }
    for (size_t i = 0; i < 3; i++)
    {
        drawPile.push_back(std::make_unique<DiscountCard>());
    }
    for (size_t i = 0; i < 2; i++)
    {
        drawPile.push_back(std::make_unique<ShieldCard>());
    }
    for (size_t i = 0; i < 2; i++)
    {
        drawPile.push_back(std::make_unique<TeleportCard>());
    }
    for (size_t i = 0; i < 2; i++)
    {
        drawPile.push_back(std::make_unique<LassoCard>());
    }
    for (size_t i = 0; i < 2; i++)
    {
        drawPile.push_back(std::make_unique<DemolitionCard>());
    }
}

AbilityCardDeck::AbilityCardDeck(std::vector<std::unique_ptr<AbilityCard>> drawPile,
    std::vector<std::unique_ptr<AbilityCard>> discardedPile)
    : drawPile(drawPile), discardedPile(discardedPile) {}

std::unique_ptr<AbilityCard> AbilityCardDeck::draw() {
    if (this->drawPile.empty()) {
        std::mt19937 randomizer(std::random_device{}());
        std::shuffle(this->discardedPile.begin(), this->discardedPile.end(), randomizer);
        for (int i = 0; i < this->discardedPile.size(); i++) {
            auto back = std::move(this->discardedPile.back());
            this->discardedPile.pop_back();
            this->drawPile.push_back(std::move(back));
        }
    }
    auto drawn = std::move(this->drawPile.back());
    this->drawPile.pop_back();
    drawn->setup();
    return drawn;
}

void AbilityCardDeck::discard(std::unique_ptr<AbilityCard> card) {
    this->discardedPile.push_back(std::move(card));
}