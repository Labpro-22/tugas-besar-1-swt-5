#ifndef COREDUMMY_GAME_VIEW_MODEL_HPP
#define COREDUMMY_GAME_VIEW_MODEL_HPP

#include <string>
#include <vector>

enum class TileKind {
    Street,
    Railroad,
    Utility,
    Card,
    Festival,
    Tax,
    Special
};

enum class PropertyStatusView {
    Bank,
    Owned,
    Mortgaged
};

enum class OverlayType {
    None,
    Info,
    Properties,
    Cards,
    Festival,
    Tax,
    Auction,
    SaveLoad,
    Victory
};

struct TileViewData {
    std::string code;
    std::string name;
    TileKind kind = TileKind::Special;
    std::string colorGroup = "DEFAULT";
    int price = 0;
    int ownerIndex = -1;
    PropertyStatusView status = PropertyStatusView::Bank;
    int houses = 0;
    bool hotel = false;
    int festivalMultiplier = 1;
};

struct PlayerViewData {
    std::string name;
    int money = 0;
    int position = 0;
    std::string status = "ACTIVE";
    bool isCurrent = false;
};

struct CardViewData {
    std::string title;
    std::string description;
};

struct LogEntryViewData {
    int turn = 1;
    std::string actor;
    std::string type;
    std::string detail;
};

struct OverlayViewData {
    OverlayType type = OverlayType::None;
    std::string title;
    std::vector<std::string> lines;
    std::string footer;
};

struct GameViewModel {
    std::vector<TileViewData> board;
    std::vector<PlayerViewData> players;
    std::vector<CardViewData> hand;
    std::vector<LogEntryViewData> logs;
    OverlayViewData overlay;
    int selectedTileIndex = 0;
    int currentPlayerIndex = 0;
    int currentTurn = 1;
    int maxTurn = 15;
    std::string statusLine = "Core dummy is active. UI is connected through IGameFacade.";
    std::string footerHint = "Dummy mode: actions update the view model only.";
};

#endif
