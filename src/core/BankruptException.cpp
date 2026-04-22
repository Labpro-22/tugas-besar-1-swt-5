#include "../../include/core/BankruptException.hpp"

BankruptException::BankruptException(int code, string message) 
    : GameException(message), bankruptCode(code) {}

int BankruptException::getBankruptCode() const {
    return bankruptCode;
}