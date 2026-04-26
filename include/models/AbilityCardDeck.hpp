#pragma once
#include <vector>
#include <memory>

class AbilityCard;

class AbilityCardDeck
{
private:
    std::vector<std::unique_ptr<AbilityCard>> drawPile;
    std::vector<std::unique_ptr<AbilityCard>> discardedPile;
public:
    AbilityCardDeck();
    AbilityCardDeck(std::vector<std::unique_ptr<AbilityCard>> drawPile, std::vector<std::unique_ptr<AbilityCard>> discardedPile);
    AbilityCardDeck(AbilityCardDeck& other);
    std::unique_ptr<AbilityCard> draw();
    void discard(std::unique_ptr<AbilityCard> card);
    const std::vector<std::unique_ptr<AbilityCard>>& getDrawPile() const { return drawPile; }
    const std::vector<std::unique_ptr<AbilityCard>>& getDiscardedPile() const { return discardedPile; }
};