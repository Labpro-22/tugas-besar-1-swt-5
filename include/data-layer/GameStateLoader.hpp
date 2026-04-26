#pragma once
#include <tuple>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include "../utils/LogEntry.hpp"
#include "../core/CardManager.hpp"
#include "../utils/PropertyTile.hpp"
#include "../core/Account.hpp"
#include "../core/TurnManager.hpp"
#include "FileIOException.hpp"
using namespace std;

class GameStateLoader
{
private:
    ifstream gameIfStream;
    static string trimString(const string& line);
    static int wordCounter(const string& line);
public:
    GameStateLoader(string path);
    TurnManager loadTurn();
    vector<int> loadPlayers();
    vector<PropertyTile> loadProperties();
    CardManager loadDecks();
    vector<LogEntry> loadLogs();
};
