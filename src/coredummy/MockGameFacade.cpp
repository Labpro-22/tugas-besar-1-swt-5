#include "../../include/coredummy/MockGameFacade.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <vector>
#include "coredummy/DummyBoardFactory.hpp"

namespace {
std::string sanitizeName(const std::string& value, int fallbackIndex) {
    if (!value.empty()) {
        return value;
    }
    return "Player " + std::to_string(fallbackIndex + 1);
}

std::string trimCopy(const std::string& value) {
    std::size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }
    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }
    return value.substr(first, last - first);
}
}

MockGameFacade::MockGameFacade() {
    loadDummyBoard();
    loadDemoGame();
}

const GameViewModel& MockGameFacade::getViewModel() const {
    return vm;
}

void MockGameFacade::tick(float deltaSeconds) {
    elapsedSeconds += deltaSeconds;
    (void)elapsedSeconds;
}

std::string MockGameFacade::validateNewGameSettings(const std::vector<std::string>& playerNames,
                                                    const std::string& configDirectory) const {
    const int count = static_cast<int>(playerNames.size());
    if (count < 2 || count > 4) {
        return "Jumlah pemain harus 2 sampai 4.";
    }
    std::vector<std::string> seen;
    for (int i = 0; i < count; ++i) {
        const std::string name = trimCopy(playerNames[static_cast<std::size_t>(i)]);
        if (name.empty()) {
            return "Nama pemain " + std::to_string(i + 1) + " tidak boleh kosong.";
        }
        if (name.size() > 8) {
            return "Nama pemain maksimal 8 karakter.";
        }
        if (std::find(seen.begin(), seen.end(), name) != seen.end()) {
            return "Username " + name + " harus unik.";
        }
        seen.push_back(name);
    }
    if (trimCopy(configDirectory).empty()) {
        return "Folder config tidak boleh kosong.";
    }
    return "";
}

bool MockGameFacade::startNewGame(const std::vector<std::string>& playerNames,
                                  const std::string& configDirectory) {
    std::string validationError = validateNewGameSettings(playerNames, configDirectory);
    if (!validationError.empty()) {
        vm.statusLine = validationError;
        return false;
    }

    if (vm.board.empty()) {
        loadDummyBoard();
    }

    const int count = std::max(2, std::min(4, static_cast<int>(playerNames.size())));
    vm.players.clear();
    vm.players.reserve(static_cast<std::size_t>(count));

    for (int i = 0; i < count; ++i) {
        PlayerViewData player;
        player.name = sanitizeName(playerNames[static_cast<std::size_t>(i)], i);
        player.money = 1000;
        player.position = 0;
        player.status = "ACTIVE";
        player.isCurrent = (i == 0);
        vm.players.push_back(player);
    }

    for (TileViewData& tile : vm.board) {
        tile.ownerIndex = -1;
        tile.status = PropertyStatusView::Bank;
        tile.houses = 0;
        tile.hotel = false;
        tile.festivalMultiplier = 1;
    }

    vm.currentPlayerIndex = 0;
    vm.currentTurn = 1;
    vm.selectedTileIndex = 0;
    vm.hand = {
        {"Move", "Dummy MoveCard"},
        {"Shield", "Dummy ShieldCard"},
        {"Teleport", "Dummy TeleportCard"}
    };
    vm.logs.clear();
    vm.overlay = OverlayViewData{};
    vm.statusLine = "New dummy game started. Core adapter can replace this facade later.";
    vm.footerHint = "Click a tile or press an action button. Logic is intentionally lightweight.";
    addLog("System", "NEW", "Dummy game created with " + std::to_string(count) + " players.");
    normalizeCurrentPlayer();
    return true;
}

void MockGameFacade::loadDemoGame() {
    std::vector<std::string> names = {"Kebin", "Stewart", "Gro", "Bob"};
    startNewGame(names, "config/basic");

    if (vm.board.size() >= 40) {
        const std::vector<int> sampleOwned = {1, 3, 5, 12, 15, 19, 24, 31, 37, 39};
        for (std::size_t i = 0; i < sampleOwned.size(); ++i) {
            TileViewData& tile = vm.board[static_cast<std::size_t>(sampleOwned[i])];
            if (tile.kind == TileKind::Street || tile.kind == TileKind::Railroad || tile.kind == TileKind::Utility) {
                tile.ownerIndex = static_cast<int>(i % vm.players.size());
                tile.status = PropertyStatusView::Owned;
                if (tile.kind == TileKind::Street && i % 3 == 0) {
                    tile.houses = 1 + static_cast<int>(i % 4);
                }
                if (tile.kind == TileKind::Street && i == sampleOwned.size() - 1) {
                    tile.hotel = true;
                    tile.houses = 0;
                }
            }
        }
        vm.board[24].festivalMultiplier = 2;
    }

    if (vm.players.size() >= 4) {
        vm.players[0].position = 0;
        vm.players[1].position = 10;
        vm.players[2].position = 20;
        vm.players[3].position = 30;
        vm.players[1].money = 875;
        vm.players[2].money = 1420;
        vm.players[3].money = 690;
    }

    vm.statusLine = "Demo loaded from UI dummy facade. Board data is provided by coredummy only.";
    addLog("System", "LOAD", "Demo state loaded for UI integration testing.");
    normalizeCurrentPlayer();
}

void MockGameFacade::selectTile(int index) {
    if (vm.board.empty()) {
        return;
    }
    vm.selectedTileIndex = std::max(0, std::min(index, static_cast<int>(vm.board.size()) - 1));
}

void MockGameFacade::rollDice() {
    if (vm.players.empty() || vm.board.empty()) {
        return;
    }

    PlayerViewData& player = vm.players[static_cast<std::size_t>(vm.currentPlayerIndex)];
    const int die1 = rollCounter % 6 + 1;
    const int die2 = (rollCounter * 2 + 3) % 6 + 1;
    ++rollCounter;
    const int total = die1 + die2;
    const int previous = player.position;
    player.position = (player.position + total) % static_cast<int>(vm.board.size());
    vm.selectedTileIndex = player.position;

    std::ostringstream detail;
    detail << "Rolled " << die1 << "+" << die2 << "=" << total
           << ", landed on " << vm.board[static_cast<std::size_t>(player.position)].code;
    addLog(player.name, "DICE", detail.str());

    applyDummyLandingEffect(player, previous, total);
    normalizeCurrentPlayer();
}

void MockGameFacade::advanceTurn() {
    if (vm.players.empty()) {
        return;
    }

    vm.currentPlayerIndex = (vm.currentPlayerIndex + 1) % static_cast<int>(vm.players.size());
    if (vm.currentPlayerIndex == 0) {
        ++vm.currentTurn;
    }
    normalizeCurrentPlayer();
    addLog(vm.players[static_cast<std::size_t>(vm.currentPlayerIndex)].name, "TURN", "Turn moved to active player.");
}

void MockGameFacade::buyCurrentProperty() {
    if (vm.players.empty() || vm.board.empty()) {
        return;
    }

    PlayerViewData& player = vm.players[static_cast<std::size_t>(vm.currentPlayerIndex)];
    TileViewData& tile = vm.board[static_cast<std::size_t>(player.position)];
    if ((tile.kind == TileKind::Street || tile.kind == TileKind::Railroad || tile.kind == TileKind::Utility) &&
        tile.ownerIndex < 0 && player.money >= tile.price) {
        player.money -= tile.price;
        tile.ownerIndex = vm.currentPlayerIndex;
        tile.status = PropertyStatusView::Owned;
        addLog(player.name, "BUY", "Bought " + tile.code + " for M" + std::to_string(tile.price) + ".");
    } else {
        addLog(player.name, "BUY_FAIL", "No available property to buy.");
    }
    normalizeCurrentPlayer();
}

void MockGameFacade::mortgageSelectedProperty() {
    if (vm.players.empty() || vm.board.empty()) {
        return;
    }
    TileViewData& tile = vm.board[static_cast<std::size_t>(vm.selectedTileIndex)];
    if (tile.ownerIndex == vm.currentPlayerIndex && tile.status == PropertyStatusView::Owned) {
        tile.status = PropertyStatusView::Mortgaged;
        vm.players[static_cast<std::size_t>(vm.currentPlayerIndex)].money += tile.price / 2;
        addLog(vm.players[static_cast<std::size_t>(vm.currentPlayerIndex)].name, "GADAI",
               "Mortgaged " + tile.code + ".");
        normalizeCurrentPlayer();
    } else {
        showOverlay(OverlayType::Info, "Gadai Gagal", {"Pilih properti milik pemain aktif."}, "");
    }
}

void MockGameFacade::redeemSelectedProperty() {
    if (vm.players.empty() || vm.board.empty()) {
        return;
    }
    TileViewData& tile = vm.board[static_cast<std::size_t>(vm.selectedTileIndex)];
    PlayerViewData& player = vm.players[static_cast<std::size_t>(vm.currentPlayerIndex)];
    if (tile.ownerIndex == vm.currentPlayerIndex && tile.status == PropertyStatusView::Mortgaged &&
        player.money >= tile.price) {
        player.money -= tile.price;
        tile.status = PropertyStatusView::Owned;
        addLog(player.name, "TEBUS", "Redeemed " + tile.code + ".");
        normalizeCurrentPlayer();
    } else {
        showOverlay(OverlayType::Info, "Tebus Gagal", {"Pilih properti tergadai dan pastikan uang cukup."}, "");
    }
}

void MockGameFacade::buildSelectedProperty() {
    if (vm.players.empty() || vm.board.empty()) {
        return;
    }
    TileViewData& tile = vm.board[static_cast<std::size_t>(vm.selectedTileIndex)];
    PlayerViewData& player = vm.players[static_cast<std::size_t>(vm.currentPlayerIndex)];
    const int cost = std::max(50, tile.price / 2);
    if (tile.ownerIndex == vm.currentPlayerIndex && tile.kind == TileKind::Street &&
        tile.status == PropertyStatusView::Owned && !tile.hotel && player.money >= cost) {
        player.money -= cost;
        if (tile.houses < 4) {
            ++tile.houses;
        } else {
            tile.houses = 0;
            tile.hotel = true;
        }
        addLog(player.name, "BANGUN", "Built on " + tile.code + ".");
        normalizeCurrentPlayer();
    } else {
        showOverlay(OverlayType::Info, "Bangun Gagal", {"Pilih street milik pemain aktif dan pastikan uang cukup."}, "");
    }
}

void MockGameFacade::openSelectedTileDetails() {
    if (vm.board.empty()) {
        return;
    }
    const TileViewData& tile = vm.board[static_cast<std::size_t>(vm.selectedTileIndex)];
    std::vector<std::string> lines = {
        "Code: " + tile.code,
        "Name: " + tile.name,
        "Kind: " + tileKindToText(tile.kind),
        "Color: " + tile.colorGroup,
        "Price: M" + std::to_string(tile.price),
        "Owner: " + (tile.ownerIndex >= 0 && tile.ownerIndex < static_cast<int>(vm.players.size()) ? vm.players[static_cast<std::size_t>(tile.ownerIndex)].name : std::string("BANK")),
        "Status: " + propertyStatusToText(tile.status)
    };
    showOverlay(OverlayType::Info, "Selected Tile", lines, "Dummy detail view. Replace with real Akta data later.");
}

void MockGameFacade::showCurrentPlayerProperties() {
    std::vector<std::string> lines;
    if (!vm.players.empty()) {
        const int owner = vm.currentPlayerIndex;
        lines.push_back("Owner: " + vm.players[static_cast<std::size_t>(owner)].name);
        for (const TileViewData& tile : vm.board) {
            if (tile.ownerIndex == owner) {
                lines.push_back(tile.code + " - " + tile.name + " | " + propertyStatusToText(tile.status));
            }
        }
    }
    if (lines.size() <= 1) {
        lines.push_back("No owned property in dummy state yet.");
    }
    showOverlay(OverlayType::Properties, "Current Player Properties", lines, "Real core can fill this through the same view model.");
}

void MockGameFacade::showCardPopup() {
    std::vector<std::string> lines;
    for (const CardViewData& card : vm.hand) {
        lines.push_back(card.title + " - " + card.description);
    }
    showOverlay(OverlayType::Cards, "Ability Cards", lines, "Dummy cards do not affect real game state yet.");
}

void MockGameFacade::showFestivalPopup() {
    showOverlay(OverlayType::Festival, "Festival", {
        "Festival will double rent for selected property in real core.",
        "Dummy mode marks selected property with multiplier x2.",
        "Railroad and Utility are allowed targets according to QnA."
    }, "Press X or Escape to close.");

    if (!vm.board.empty()) {
        TileViewData& tile = vm.board[static_cast<std::size_t>(vm.selectedTileIndex)];
        if (tile.kind == TileKind::Street || tile.kind == TileKind::Railroad || tile.kind == TileKind::Utility) {
            tile.festivalMultiplier = std::min(8, tile.festivalMultiplier * 2);
        }
    }
}

void MockGameFacade::showTaxPopup() {
    showOverlay(OverlayType::Tax, "Tax", {
        "PPH flat: M150",
        "PPH percent: 10% of total wealth",
        "PBM flat: M200",
        "Dummy mode only previews the panel."
    }, "Tax payment should be delegated to real core logic.");
}

void MockGameFacade::showAuctionPopup() {
    showOverlay(OverlayType::Auction, "Auction", {
        "Auction order starts after the triggering player.",
        "Pass does not remove a player from the auction.",
        "At least one bid is required before auction can finish.",
        "Dummy mode does not run bidding yet."
    }, "Use this panel as UI placeholder for real auction flow.");
}

void MockGameFacade::showSavePopup() {
    showOverlay(OverlayType::SaveLoad, "Save / Load", {
        "Save and load are placeholders in coredummy.",
        "QnA removes the in-game MUAT command.",
        "Load should happen from the initial menu.",
        "The UI can keep this panel as a debug shortcut for now."
    }, "Connect this to data-layer saver/loader later.");
}

void MockGameFacade::showVictoryPopup() {
    std::vector<std::string> lines;
    std::vector<PlayerViewData> ranking = vm.players;
    std::sort(ranking.begin(), ranking.end(), [](const PlayerViewData& left, const PlayerViewData& right) {
        return left.money > right.money;
    });
    for (const PlayerViewData& player : ranking) {
        lines.push_back(player.name + " | Money M" + std::to_string(player.money));
    }
    showOverlay(OverlayType::Victory, "Victory Preview", lines, "Dummy winner is based on cash only.");
}

void MockGameFacade::closeOverlay() {
    vm.overlay = OverlayViewData{};
}

void MockGameFacade::loadDummyBoard() {
    vm.maxTurn = 15;
    salary = 200;
    vm.board = DummyBoardFactory::createDefaultBoard();
}

void MockGameFacade::addLog(const std::string& actor, const std::string& type, const std::string& detail) {
    LogEntryViewData entry;
    entry.turn = vm.currentTurn;
    entry.actor = actor;
    entry.type = type;
    entry.detail = detail;
    vm.logs.insert(vm.logs.begin(), entry);
    if (vm.logs.size() > 12) {
        vm.logs.pop_back();
    }
}

void MockGameFacade::showOverlay(OverlayType type, const std::string& title, const std::vector<std::string>& lines, const std::string& footer) {
    vm.overlay.type = type;
    vm.overlay.title = title;
    vm.overlay.lines = lines;
    vm.overlay.footer = footer;
}

void MockGameFacade::normalizeCurrentPlayer() {
    if (vm.players.empty()) {
        vm.currentPlayerIndex = 0;
        return;
    }
    vm.currentPlayerIndex = std::max(0, std::min(vm.currentPlayerIndex, static_cast<int>(vm.players.size()) - 1));
    for (std::size_t i = 0; i < vm.players.size(); ++i) {
        vm.players[i].isCurrent = (static_cast<int>(i) == vm.currentPlayerIndex);
    }
    const PlayerViewData& player = vm.players[static_cast<std::size_t>(vm.currentPlayerIndex)];
    vm.statusLine = player.name + " is playing. Dummy facade is feeding the UI.";
}

void MockGameFacade::applyDummyLandingEffect(PlayerViewData& player, int previousPosition, int diceTotal) {
    (void)diceTotal;
    if (vm.board.empty()) {
        return;
    }

    if (player.position < previousPosition) {
        player.money += salary;
        addLog(player.name, "GO", "Passed GO and received M" + std::to_string(salary) + ".");
    }

    TileViewData& tile = vm.board[static_cast<std::size_t>(player.position)];
    if (tile.kind == TileKind::Street || tile.kind == TileKind::Railroad || tile.kind == TileKind::Utility) {
        if (tile.ownerIndex < 0) {
            tile.ownerIndex = vm.currentPlayerIndex;
            tile.status = PropertyStatusView::Owned;
            if (tile.price > 0 && player.money >= tile.price) {
                player.money -= tile.price;
                addLog(player.name, "BUY", "Dummy bought " + tile.code + " for M" + std::to_string(tile.price) + ".");
            } else {
                addLog(player.name, "CLAIM", "Dummy claimed " + tile.code + ".");
            }
        } else if (tile.ownerIndex != vm.currentPlayerIndex && tile.ownerIndex < static_cast<int>(vm.players.size())) {
            const int rent = std::max(10, tile.price / 10) * tile.festivalMultiplier;
            player.money -= rent;
            vm.players[static_cast<std::size_t>(tile.ownerIndex)].money += rent;
            addLog(player.name, "RENT", "Dummy paid M" + std::to_string(rent) + " to " + vm.players[static_cast<std::size_t>(tile.ownerIndex)].name + ".");
        }
    } else if (tile.kind == TileKind::Tax) {
        const int tax = tile.code == "PBM" ? 200 : 150;
        player.money -= tax;
        addLog(player.name, "TAX", "Dummy paid M" + std::to_string(tax) + " on " + tile.code + ".");
    } else if (tile.kind == TileKind::Festival) {
        addLog(player.name, "FEST", "Festival tile reached. Use Festival button to mark a property.");
    } else if (tile.kind == TileKind::Card) {
        addLog(player.name, "CARD", "Dummy drew a card from " + tile.code + ".");
    } else if (tile.code == "PPJ") {
        player.position = 10;
        vm.selectedTileIndex = 10;
        player.status = "JAILED";
        addLog(player.name, "JAIL", "Dummy moved player to jail.");
    }
}

std::string MockGameFacade::tileKindToText(TileKind kind) const {
    switch (kind) {
        case TileKind::Street: return "Street";
        case TileKind::Railroad: return "Railroad";
        case TileKind::Utility: return "Utility";
        case TileKind::Card: return "Card";
        case TileKind::Festival: return "Festival";
        case TileKind::Tax: return "Tax";
        case TileKind::Special: return "Special";
    }
    return "Tile";
}

std::string MockGameFacade::propertyStatusToText(PropertyStatusView status) const {
    switch (status) {
        case PropertyStatusView::Bank: return "BANK";
        case PropertyStatusView::Owned: return "OWNED";
        case PropertyStatusView::Mortgaged: return "MORTGAGED";
    }
    return "UNKNOWN";
}
