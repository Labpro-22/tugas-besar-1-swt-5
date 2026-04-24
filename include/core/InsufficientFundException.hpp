#pragma once
#include "GameException.hpp"

class InsufficientFundException : public GameException
{
private:
    int deficit;
public:
    InsufficientFundException(int deficit);
    int getDeficit() const;
};