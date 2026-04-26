#include "../../include/utils/Logger.hpp"
#include <string>

void Logger::log(int turn, string username, string actionType, string detail)
{
    entries.push_back(LogEntry(turn, username, actionType, detail));
}

void Logger::printLog()
{
    if (entries.empty())
    {
        cout << "No log entries." << endl;
        return;
    }

    for (const LogEntry& entry : entries)
    {
        cout << entry.toString() << endl;
    }
}

vector<LogEntry> Logger::getEntries()
{
    return entries;
}

void Logger::clear()
{
    entries.clear();
}

std::string Logger::serialize() const
{
    std::string result;
    for (const LogEntry& entry : entries)
    {
        result += std::to_string(entry.getTurn())
               + " " + entry.getUsername()
               + " " + entry.getActionType()
               + " " + entry.getDetail()
               + "\n";
    }
    return result;
}
