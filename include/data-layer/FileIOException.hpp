#pragma once
#include "../core/GameException.hpp"

class FileIOException : public GameException 
{
public:
    explicit FileIOException(string ErrorMessage);
};
