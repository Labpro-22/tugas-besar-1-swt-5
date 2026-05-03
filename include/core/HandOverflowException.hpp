#pragma once

#include "GameException.hpp"

class HandOverflowException : public GameException {
public:
    explicit HandOverflowException(string message);
};
