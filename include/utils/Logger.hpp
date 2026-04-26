#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "LogEntry.hpp"
using namespace std;

class Logger
{
private:
    vector<LogEntry> entries;
public:
    void log(int turn, string username, string actionType, string detail);
    void printLog();
    vector<LogEntry> getEntries();
    void clear();
    std::string serialize() const;
};