#include "../../include/core/InsufficientFundException.hpp"

InsufficientFundException::InsufficientFundException(int deficit) 
    : GameException("uang tidak cukup"), deficit(deficit) {}
int InsufficientFundException::getDeficit() const {
    return deficit;
}