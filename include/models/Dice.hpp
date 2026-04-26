#pragma once
#include <random>

class Dice
{
private:
    std::mt19937 randomizer;
    std::uniform_int_distribution<> picker;
public:
    Dice() : randomizer(std::random_device{}()), picker(1, 6) {}
    std::pair<int, int> roll() {
        return {picker(randomizer), picker(randomizer)};
    }
    bool isDouble(std::pair<int, int> result) {
        auto [num1, num2] = result;
        return num1 == num2;
    }
};
