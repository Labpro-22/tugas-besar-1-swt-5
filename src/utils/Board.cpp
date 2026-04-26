#include "utils/Board.hpp"
#include "utils/RailroadTile.hpp"
#include "utils/SpecialTile.hpp"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

Board::~Board() {
    for (Tile* tile : tilesVector) {
        delete tile;
    }
}

void Board::addTile(Tile* tile) {
    if (tile == nullptr) {
        throw std::invalid_argument("Cannot add null tile to board.");
    }

    const std::string code = tile->getCode();
    codeToIndices[code].push_back(static_cast<int>(tilesVector.size()));
    tilesVector.push_back(tile);
}

Tile* Board::getTileByIndex(int index) const {
    if (tilesVector.empty()) {
        throw std::out_of_range("Board has no tiles.");
    }

    const int boardSize = static_cast<int>(tilesVector.size());
    int normalizedIndex = index % boardSize;
    if (normalizedIndex < 0) {
        normalizedIndex += boardSize;
    }

    return tilesVector[normalizedIndex];
}

Tile* Board::getTileByCode(const std::string& code) const {
    const auto it = codeToIndices.find(code);
    if (it == codeToIndices.end() || it->second.empty()) {
        throw std::out_of_range("Unknown tile code: " + code);
    }

    return tilesVector[it->second.front()];
}

std::vector<int> Board::getTileIndicesByCode(const std::string& code) const {
    const auto it = codeToIndices.find(code);
    if (it == codeToIndices.end()) {
        return {};
    }
    return it->second;
}

int Board::calculateNewPosition(int pos, int step, bool& passedGo) const {
    if (tilesVector.empty()) {
        throw std::out_of_range("Board has no tiles.");
    }

    const int boardSize = static_cast<int>(tilesVector.size());
    const int rawPosition = pos + step;
    int newPos = rawPosition % boardSize;
    if (newPos < 0) {
        newPos += boardSize;
    }

    passedGo = (step > 0 && rawPosition > boardSize);
    return newPos;
}

int Board::getStepsToNearestRailroad(int fromPos) const {
    if (tilesVector.empty()) {
        return 0;
    }

    for (int step = 1; step < static_cast<int>(tilesVector.size()); ++step) {
        Tile* candidate = getTileByIndex(fromPos + step);
        if (dynamic_cast<RailroadTile*>(candidate) != nullptr) {
            return step;
        }
        Tile* negativeCandidate = getTileByIndex(fromPos - step);
        if (dynamic_cast<RailroadTile*>(negativeCandidate) != nullptr) {
            return -step;
        }
    }

    return 0;
}

int Board::getJailIndex() const {
    for (size_t i = 0; i < tilesVector.size(); i++)
    {
        Tile* candidate = getTileByIndex(i);
        SpecialTile* cast = dynamic_cast<SpecialTile*>(candidate);
        if (cast != nullptr) {
            if (cast->isJail()) return i;
        }
    }
    return -1;
}

const std::vector<Tile*>& Board::getTiles() const {
    return tilesVector;
}

int Board::size() const {
    return static_cast<int>(tilesVector.size());
}

void Board::printBoard(
    const std::map<int, std::vector<std::string>>& playerMarkers,
    const std::string& turnInfo
) const {
    if (!turnInfo.empty()) {
        std::cout << turnInfo << '\n';
    }

    if (tilesVector.empty()) {
        std::cout << "[board kosong]\n";
        return;
    }

    for (int i = 0; i < static_cast<int>(tilesVector.size()); ++i) {
        std::ostringstream cell;
        cell << "[" << std::setw(2) << std::setfill('0') << tilesVector[i]->getId()
             << "] " << tilesVector[i]->getCode();

        const auto markerIt = playerMarkers.find(i);
        if (markerIt != playerMarkers.end() && !markerIt->second.empty()) {
            cell << " {";
            for (std::size_t markerIndex = 0; markerIndex < markerIt->second.size(); ++markerIndex) {
                if (markerIndex > 0) {
                    cell << ",";
                }
                cell << markerIt->second[markerIndex];
            }
            cell << "}";
        }

        std::cout << std::left << std::setw(20) << cell.str();
        if ((i + 1) % 5 == 0 || i + 1 == static_cast<int>(tilesVector.size())) {
            std::cout << '\n';
        }
    }
}
