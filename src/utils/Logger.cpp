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


void Logger::printLog(int count)
{
    if (count <= 0)
    {
        printLog();
        return;
    }

    if (entries.empty())
    {
        cout << "No log entries." << endl;
        return;
    }

    size_t start = 0;
    if (entries.size() > static_cast<size_t>(count))
    {
        start = entries.size() - static_cast<size_t>(count);
    }

    for (size_t i = start; i < entries.size(); ++i)
    {
        cout << entries[i].toString() << endl;
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
