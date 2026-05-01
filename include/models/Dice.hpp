#pragma once
#include <random>
#include <stdexcept>
#include <utility>

class Dice
{
private:
    std::mt19937 randomizer;
    std::uniform_int_distribution<> picker;
    bool manualMode = false;
    std::pair<int,int> manualResult = {1, 1};
public:
    Dice() : randomizer(std::random_device{}()), picker(1, 6) {}
    std::pair<int, int> roll() {
        if (manualMode) {
            manualMode = false;
            return manualResult;
        }
        return {picker(randomizer), picker(randomizer)};
    }
    void setManual(int d1, int d2) {
        if (d1 < 1 || d1 > 6 || d2 < 1 || d2 > 6) {
            throw std::invalid_argument("Nilai dadu harus berada di antara 1 sampai 6.");
        }

        manualResult = {d1, d2};
        manualMode = true;
    }
    bool isDouble(std::pair<int, int> result) {
        auto [num1, num2] = result;
        return num1 == num2;
    }
};
