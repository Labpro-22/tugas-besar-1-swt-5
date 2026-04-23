#pragma once

#include <string>
#include <exception>

using namespace std;

class GameException : public exception {
protected:
    string message;

public:
    explicit GameException(string message);
    virtual ~GameException() = default;

    string getMessage() const;
    virtual const char* what() const noexcept override;
};