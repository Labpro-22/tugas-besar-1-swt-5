#pragma once
#include <iostream>
#include <string>
using namespace std;

class LogEntry
{
private:
    int turn;
    string username;
    string actionType;
    string detail;
public:
    int getTurn() const { return turn; }
    std::string getUsername() const { return username; }
    std::string getActionType() const { return actionType; }
    std::string getDetail() const { return detail; }string toString();
};