#ifndef BOARD_HPP
#define BOARD_HPP

#include <map>
#include <string>
#include <vector>
#include "Tile.hpp"

class Player; // Kalau udah ada nanti tambahkan di include

class Board {
private:
    std::vector<Tile*> tilesVector;
    std::map<std::string, int> codeToIndex;

public:
    Board() = default;
    ~Board();

    void addTile(Tile* tile);

    Tile* getTileByIndex(int index) const;
    Tile* getTileByCode(const std::string& code) const;

    int calculateNewPosition(int pos, int step, bool& passedGo) const;

    int getStepsToNearestRailroad(int fromPos) const;

    int getJailIndex() const;

    const std::vector<Tile*>& getTiles() const;
    int size() const;

    void printBoard(
        const std::map<int, std::vector<std::string>>& playerMarkers = {},
        const std::string& turnInfo = ""
    ) const;
    
    std::vector<PropertyTile*>& getPropertyTiles();
};

#endif