#pragma once
#include <string>
#include <functional>

class Player;
class Game;

class DrawCard
{
protected:
    std::string description;
    std::function<void(Player*, Game*)> effect;
public:
    DrawCard() : description("Draw Card"), effect([](Player* p, Game* g){}) {}
    DrawCard(std::string description, std::function<void(Player*, Game*)> effect) : description(description), effect(effect) {}
    virtual ~DrawCard() = default;
    void apply(Player* target, Game* gameState) {
        effect(target, gameState);
    }
    std::string getDescription() const {
        return description;
    }
};
