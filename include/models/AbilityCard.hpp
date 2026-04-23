#pragma once
#include <string>
#include <random>

class Player;
class Game;

class AbilityCard
{
protected:
    std::string name;
    std::string description;
public:
    AbilityCard(std::string name, std::string description) : name(name), description(description) {}
    virtual void setup() {}
    bool canUse(Player* target, Game* game) const;
    virtual void use(Player* target, Game* game) = 0;
    std::string getName() const {
        return name;
    }
    std::string serialize() const;
};