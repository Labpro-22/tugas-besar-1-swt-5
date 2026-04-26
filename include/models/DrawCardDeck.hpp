#pragma once
#include <vector>
#include <random>

template <class T>
class DrawCardDeck
{
private:
    std::vector<T*> pile;
    std::mt19937 shuffler;
    std::uniform_int_distribution<> picker;
public:
    DrawCardDeck() : pile({}) {}
    DrawCardDeck(std::vector<T*> pile) : pile(pile), shuffler(std::random_device{}()), picker(0, (int)pile.size() - 1){}
    T* draw() {
        return pile[picker(shuffler)];
    }
    const std::vector<T*>& getPile() const { return pile; }
};
