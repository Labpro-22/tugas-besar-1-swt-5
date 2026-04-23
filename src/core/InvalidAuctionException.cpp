#include "../../include/core/InvalidActionException.hpp"

InvalidActionException::InvalidActionException(string message) 
    : GameException(message) {}