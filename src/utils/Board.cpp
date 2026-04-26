#include "utils/Board.hpp"
#include "models/Player.hpp"
#include "utils/CardTile.hpp"
#include "utils/FestivalTile.hpp"
#include "utils/PropertyTile.hpp"
#include "utils/RailroadTile.hpp"
#include "utils/SpecialTile.hpp"
#include "utils/StreetTile.hpp"
#include "utils/TaxTile.hpp"
#include "utils/UtilityTile.hpp"

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    const std::string RESET = "\033[0m";
    const std::string BOLD = "\033[1m";
    const std::string DIM = "\033[2m";

    const int CELL_INNER_WIDTH = 10;
    const int CELL_TOTAL_WIDTH = CELL_INNER_WIDTH + 2;
    const int SIDE_ROWS = 9;
    const int BOARD_COLS = 11;

    std::string ansiForGroup(const std::string& group) {
        if (group == "COKLAT") return "\033[38;5;130m";
        if (group == "BIRU_MUDA") return "\033[38;5;153m";
        if (group == "MERAH_MUDA") return "\033[95m";
        if (group == "ORANGE") return "\033[38;5;208m";
        if (group == "MERAH") return "\033[91m";
        if (group == "KUNING") return "\033[93m";
        if (group == "HIJAU") return "\033[92m";
        if (group == "BIRU_TUA") return "\033[94m";
        if (group == "ABU_ABU") return "\033[90m";
        return "\033[97m";
    }

    std::string categoryCode(const Tile* tile) {
        if (const StreetTile* st = dynamic_cast<const StreetTile*>(tile)) {
            const std::string group = st->getColorGroup();
            if (group == "COKLAT") return "CK";
            if (group == "BIRU_MUDA") return "BM";
            if (group == "MERAH_MUDA") return "PK";
            if (group == "ORANGE") return "OR";
            if (group == "MERAH") return "MR";
            if (group == "KUNING") return "KN";
            if (group == "HIJAU") return "HJ";
            if (group == "BIRU_TUA") return "BT";
            return "ST";
        }
        if (dynamic_cast<const RailroadTile*>(tile) != nullptr) return "ST";
        if (dynamic_cast<const UtilityTile*>(tile) != nullptr) return "UT";
        if (dynamic_cast<const CardTile*>(tile) != nullptr) return "KR";
        if (dynamic_cast<const TaxTile*>(tile) != nullptr) return "PJ";
        if (dynamic_cast<const FestivalTile*>(tile) != nullptr) return "FS";
        if (dynamic_cast<const SpecialTile*>(tile) != nullptr) return "SP";
        return "DF";
    }

    std::string ansiForTile(const Tile* tile) {
        if (const StreetTile* st = dynamic_cast<const StreetTile*>(tile)) {
            return ansiForGroup(st->getColorGroup());
        }
        if (dynamic_cast<const UtilityTile*>(tile) != nullptr) return ansiForGroup("ABU_ABU");
        if (dynamic_cast<const RailroadTile*>(tile) != nullptr) return "\033[96m";
        if (dynamic_cast<const CardTile*>(tile) != nullptr) return "\033[36m";
        if (dynamic_cast<const TaxTile*>(tile) != nullptr) return "\033[91m";
        if (dynamic_cast<const FestivalTile*>(tile) != nullptr) return "\033[35m";
        if (dynamic_cast<const SpecialTile*>(tile) != nullptr) return "\033[97m";
        return "\033[97m";
    }

    std::string statusText(const Tile* tile) {
        const PropertyTile* property = dynamic_cast<const PropertyTile*>(tile);
        if (property == nullptr) {
            if (dynamic_cast<const CardTile*>(tile) != nullptr) return "Kartu";
            if (dynamic_cast<const TaxTile*>(tile) != nullptr) return "Pajak";
            if (dynamic_cast<const FestivalTile*>(tile) != nullptr) return "Festival";
            if (dynamic_cast<const SpecialTile*>(tile) != nullptr) return "Spesial";
            return "-";
        }

        if (property->getStatus() == BANK || property->getOwner() == nullptr) {
            return "Bank";
        }

        std::string owner = "P" + std::to_string(property->getOwner()->getId() + 1);
        if (property->getStatus() == MORTGAGED) {
            return "Gadai:" + owner;
        }

        std::string text = "Own:" + owner;
        if (const StreetTile* street = dynamic_cast<const StreetTile*>(tile)) {
            if (street->hasHotelBuilt()) {
                text += " HT";
            } else if (street->getHouseCount() > 0) {
                text += " H" + std::to_string(street->getHouseCount());
            }
        }
        if (property->getFestivalMultiplier() > 1) {
            text += " x" + std::to_string(property->getFestivalMultiplier());
        }
        return text;
    }

    int visibleLength(const std::string& text) {
        int length = 0;
        bool inEscape = false;
        for (char ch : text) {
            if (ch == '\033') {
                inEscape = true;
                continue;
            }
            if (inEscape) {
                if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
                    inEscape = false;
                }
                continue;
            }
            ++length;
        }
        return length;
    }

    std::string truncateVisible(const std::string& text, int maxLen) {
        std::string output;
        int length = 0;
        bool inEscape = false;
        for (std::size_t i = 0; i < text.size(); ++i) {
            char ch = text[i];
            if (ch == '\033') {
                inEscape = true;
                output += ch;
                continue;
            }
            if (inEscape) {
                output += ch;
                if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
                    inEscape = false;
                }
                continue;
            }
            if (length >= maxLen) break;
            output += ch;
            ++length;
        }
        if (inEscape) output += RESET;
        return output;
    }

    std::string padVisible(std::string text, int width) {
        if (visibleLength(text) > width) {
            text = truncateVisible(text, width);
        }
        return text + std::string(std::max(0, width - visibleLength(text)), ' ');
    }
    std::string centerVisible(std::string text, int width) {
    if (visibleLength(text) > width) {
        text = truncateVisible(text, width);
    }

    int space = width - visibleLength(text);
    int left = space / 2;
    int right = space - left;

    return std::string(left, ' ') + text + std::string(right, ' ');
}
    std::vector<std::string> splitLines(const std::string& text) {
        std::vector<std::string> lines;
        std::stringstream stream(text);
        std::string line;
        while (std::getline(stream, line)) {
            lines.push_back(line);
        }
        if (lines.empty()) lines.push_back("");
        return lines;
    }

    std::string playerMarkerText(const std::map<int, std::vector<std::string>>& playerMarkers, int index) {
        auto markerIt = playerMarkers.find(index);
        if (markerIt == playerMarkers.end() || markerIt->second.empty()) {
            return "";
        }

        std::string text = "B:";
        for (const std::string& marker : markerIt->second) {
            text += marker;
        }
        return text;
    }

    std::array<std::string, 5> makeCell(const Tile* tile,
                                        int index,
                                        const std::map<int, std::vector<std::string>>& playerMarkers) {
        const std::string color = ansiForTile(tile);
        const std::string border = color + "+" + std::string(CELL_INNER_WIDTH, '-') + "+" + RESET;

        std::string line1 = "[" + categoryCode(tile) + "] " + tile->getCode();
        std::string line2 = statusText(tile);
        std::string line3 = playerMarkerText(playerMarkers, index);

        return {
            border,
            color + "|" + RESET + padVisible(line1, CELL_INNER_WIDTH) + color + "|" + RESET,
            color + "|" + RESET + padVisible(line2, CELL_INNER_WIDTH) + color + "|" + RESET,
            color + "|" + RESET + padVisible(line3, CELL_INNER_WIDTH) + color + "|" + RESET,
            border
        };
    }

    void printLine(const std::string& left,
                   const std::string& middle,
                   const std::string& right) {
        std::cout << left << middle << right << '\n';
    }

    std::vector<std::string> buildCenterLines(const std::string& turnInfo) {
        std::vector<std::string> lines;
        lines.push_back("========================================");
        lines.push_back("NIMONSPOLI                              ");
        lines.push_back("========================================");

        for (const std::string& line : splitLines(turnInfo)) {
            if (!line.empty()) lines.push_back(line);
        }

        lines.push_back("----------------------------------------");
        lines.push_back("LEGENDA");
        lines.push_back("B:P1P2 = posisi bidak pemain");
        lines.push_back("Own:P1 = properti milik P1");
        lines.push_back("Bank = belum dimiliki");
        lines.push_back("Gadai:P1 = properti digadaikan");
        lines.push_back("H1-H4 = rumah, HT = hotel");
        lines.push_back("x2/x4 = efek festival");
        lines.push_back("----------------------------------------");
        lines.push_back("WARNA");
        lines.push_back("CK=Coklat  BM=Biru Muda  PK=Pink");
        lines.push_back("OR=Orange  MR=Merah      KN=Kuning");
        lines.push_back("HJ=Hijau   BT=Biru Tua   UT=Utilitas");
        lines.push_back("KR=Kartu   PJ=Pajak      FS=Festival");
        lines.push_back("SP=Spesial ST=Stasiun");
        lines.push_back("========================================");
        return lines;
    }
}

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

    passedGo = (step > 0 && rawPosition >= boardSize);
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
    if (tilesVector.empty()) {
        std::cout << "[board kosong]\n";
        return;
    }

    if (static_cast<int>(tilesVector.size()) != 40) {
        for (int i = 0; i < static_cast<int>(tilesVector.size()); ++i) {
            const Tile* tile = tilesVector[i];
            std::cout << "[" << std::setw(2) << std::setfill('0') << tile->getId()
                      << "] " << tile->getCode() << " " << statusText(tile);

            const std::string markers = playerMarkerText(playerMarkers, i);
            if (!markers.empty()) std::cout << " " << markers;
            std::cout << '\n';
        }
        return;
    }

    const auto topLeft = makeCell(tilesVector[20], 20, playerMarkers);
    std::array<std::array<std::string, 5>, 9> leftCells;
    std::array<std::array<std::string, 5>, 9> rightCells;
    std::array<std::array<std::string, 5>, 11> topCells;
    std::array<std::array<std::string, 5>, 11> bottomCells;

    for (int col = 0; col < BOARD_COLS; ++col) {
        topCells[col] = makeCell(tilesVector[20 + col], 20 + col, playerMarkers);
        bottomCells[col] = makeCell(tilesVector[10 - col], 10 - col, playerMarkers);
    }
    for (int row = 0; row < SIDE_ROWS; ++row) {
        leftCells[row] = makeCell(tilesVector[19 - row], 19 - row, playerMarkers);
        rightCells[row] = makeCell(tilesVector[31 + row], 31 + row, playerMarkers);
    }

    for (int line = 0; line < 5; ++line) {
        for (int col = 0; col < BOARD_COLS; ++col) {
            std::cout << topCells[col][line];
        }
        std::cout << '\n';
    }

    const int centerWidth = CELL_TOTAL_WIDTH * 9;
    const int centerHeight = SIDE_ROWS * 5;

    const std::vector<std::string> centerLines = buildCenterLines(turnInfo);

    int topPadding = (centerHeight - static_cast<int>(centerLines.size())) / 2;
    if (topPadding < 0) topPadding = 0;

    for (int row = 0; row < SIDE_ROWS; ++row) {
        for (int line = 0; line < 5; ++line) {
            int currentLine = row * 5 + line;
            int contentIndex = currentLine - topPadding;

            std::string center;

            if (contentIndex >= 0 && contentIndex < static_cast<int>(centerLines.size())) {
                center = centerVisible(centerLines[static_cast<std::size_t>(contentIndex)], centerWidth);
            } else {
                center = std::string(centerWidth, ' ');
            }

            printLine(leftCells[row][line], center, rightCells[row][line]);
        }
    }

    for (int line = 0; line < 5; ++line) {
        for (int col = 0; col < BOARD_COLS; ++col) {
            std::cout << bottomCells[col][line];
        }
        std::cout << '\n';
    }
}
