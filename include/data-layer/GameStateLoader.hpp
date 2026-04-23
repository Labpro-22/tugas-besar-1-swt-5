#pragma once
#include <tuple>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
// #include "../core/LogEntry.hpp"
#include "../core/CardManager.hpp"
// #include "../model/PropertyTile.hpp"
#include "../core/Account.hpp"
using namespace std;

class GameStateLoader
{
private:
    ifstream gameIfStream;
public:
    GameStateLoader(string path);
    ifstream getIfstream();
    vector<Player> loadPlayers();
    // vector<PropertyTile> loadProperties();
    CardManager loadDecks();
    // vector<LogEntry> loadLogs();
};
