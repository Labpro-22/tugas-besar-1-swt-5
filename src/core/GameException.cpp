#include "../../include/core/GameException.hpp"

GameException::GameException(string message) : message(message) {}

string GameException::getMessage() const {
    return message;
}

const char* GameException::what() const noexcept {
    return message.c_str(); 
}