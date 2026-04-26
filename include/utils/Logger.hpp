#pragma once
#include <string>
#include <iostream>
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
    void serialized();
};