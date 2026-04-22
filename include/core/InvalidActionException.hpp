#pragma once

#include "GameException.hpp"

class InvalidActionException : public GameException {
public:
    explicit InvalidActionException(string message);
};