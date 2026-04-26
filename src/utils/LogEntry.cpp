#include "../../include/utils/LogEntry.hpp"

LogEntry::LogEntry()
{
    this->turn = 0;
    this->username = "";
    this->actionType = "";
    this->detail = "";
}

LogEntry::LogEntry(int turn, string username, string actionType, string detail)
{
    this->turn = turn;
    this->username = username;
    this->actionType = actionType;
    this->detail = detail;
}

int LogEntry::getTurn() const
{
    return turn;
}

string LogEntry::getUsername() const
{
    return username;
}

string LogEntry::getActionType() const
{
    return actionType;
}

string LogEntry::getDetail() const
{
    return detail;
}

string LogEntry::toString() const
{
    return "Turn " + to_string(turn) + " | " + username + " | " + actionType + " | " + detail;
}