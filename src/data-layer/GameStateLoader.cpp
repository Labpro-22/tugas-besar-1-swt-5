// #include "../../include/data-layer/GameStateLoader.hpp"

// GameStateLoader::GameStateLoader(string path)
// {
//     this->gameIfStream.open(path);
//     if (!this->gameIfStream.is_open())
//     {
//         throw FileIOException("Gagal membuka file: " + path + "\n");
//     }
// }

// int GameStateLoader::wordCounter(const string &line)
// {
//     int count = 0;
//     bool inWord;
//     for (char c : line)
//     {
//         if (isspace(c))
//         {
//             inWord = false;
//         }
//         else if (!inWord)
//         {
//             inWord = true;
//             count++;
//         }
//     }
//     return count;
// }

// string GameStateLoader::trimString(const string &line)
// {
//     const string whitespace = " \t\v\n\r\f";
//     size_t start = line.find_first_not_of(whitespace);
//     if (start == string::npos)
//         return "";
//     size_t end = line.find_last_not_of(whitespace);
//     return line.substr(start, end - start + 1);
// }

// TurnManager GameStateLoader::loadTurn()
// {
//     string line, token, username;
//     int currentPlayerIndex = -1;
//     vector<string> usernamesVect;
//     bool valid = false;
//     int ctr, tChk, tNow, tMax;
//     istringstream iss(line);
//     if (getline(this->gameIfStream, line))
//     {
//         line = trimString(line);
//         if (wordCounter(line) == 2)
//         {
//             if (iss >> token)
//             {
//                 ctr = 0;
//                 tChk = stoi(token);
//                 if (tChk == 0 && token.length() == 1)
//                 {
//                     tNow = tChk;
//                     valid = true;
//                 }
//                 else
//                 {
//                     while (tChk > 0)
//                     {
//                         tChk /= 10;
//                         ctr++;
//                     }
//                     if (ctr == token.length())
//                     {
//                         tNow = tChk;
//                         valid = true;
//                     }
//                 }
//                 if (valid)
//                 {
//                     valid = false;
//                     if (iss >> token)
//                     {
//                         ctr = 0;
//                         tChk = stoi(token);
//                         if (tChk == 0 && token.length() == 1)
//                         {
//                             tMax = tChk;
//                             valid = true;
//                         }
//                         else
//                         {
//                             while (tChk > 0)
//                             {
//                                 tMax /= 10;
//                                 ctr++;
//                             }
//                             if (ctr == token.length())
//                             {
//                                 tMax = tChk;
//                                 valid = true;
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//         if (getline(this->gameIfStream, line))
//         {
//             if (wordCounter(line) == 1)
//             {
//                 istringstream iss(line);
//                 if (iss >> token)
//                 {
//                     int jumlahPlayer = stoi(token);
//                     int accountCtr = 0;
//                     while (getline(this->gameIfStream, line) && accountCtr <= jumlahPlayer)
//                     {
//                         line = trimString(line);
//                         if (wordCounter(line) == 4)
//                         {
//                             istringstream usEx(line);
//                             if (usEx >> username)
//                             {
//                                 usernamesVect.push_back(username);
//                             }
//                             accountCtr++;
//                         }
//                     }
//                     if (getline(this->gameIfStream, line))
//                     {
//                         line = trimString(line);
//                         int nKartu = stoi(line);
//                         for (int i = 0; i < nKartu; i++)
//                             getline(this->gameIfStream, line);
//                         if (getline(this->gameIfStream, line))
//                         {
//                             line = trimString(line);
//                             if (wordCounter(line) == 1)
//                             {
//                                 for (size_t i = 0; i < usernamesVect.size(); i++)
//                                 {
//                                     if (usernamesVect[i].compare(line) == 0)
//                                     {
//                                         currentPlayerIndex = i;
//                                         break;
//                                     }
//                                 }
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//         TurnManager turnManager(vector<int>{0, 1, 2, 3, 4}, tMax);
//         turnManager.setCurrentPlayerIndex(currentPlayerIndex);
//         turnManager.setCurrentTurn(tNow);
//     }
// }

// vector<Player> GameStateLoader::loadPlayers()
// {
    
// }

// vector<PropertyTile> GameStateLoader::loadProperties()
// {
// }

// CardManager GameStateLoader::loadDecks()
// {
// }

// vector<LogEntry> loadLogs()
// {
// }