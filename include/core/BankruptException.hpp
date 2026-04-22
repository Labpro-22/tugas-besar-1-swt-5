#pragma once

#include "GameException.hpp"

class BankruptException : public GameException {
private:
    int bankruptCode;

public:
    BankruptException(int code, string message);
    int getBankruptCode() const;
};