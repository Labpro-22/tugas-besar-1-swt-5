#pragma once
#include <stdexcept>

class AccountException : std::runtime_error
{
public:
    AccountException(std::string message) : std::runtime_error(message.c_str()) {}
};

class AccountNotFoundException : AccountException
{
public:
    AccountNotFoundException() : AccountException("account not registered") {}
};

class UsernameUnavailableException : AccountException
{
public:
    UsernameUnavailableException() : AccountException("this username already exists") {}
};

class WrongPasswordException : AccountException
{
public:
    WrongPasswordException() : AccountException("password doesn't match") {}
};