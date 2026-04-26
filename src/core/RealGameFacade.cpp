#include "../../include/core/RealGameFacade.hpp"
#include "../../include/core/Game.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/utils/Board.hpp"
#include "../../include/utils/Tile.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/utils/RailroadTile.hpp"
#include "../../include/utils/UtilityTile.hpp"
#include "../../include/utils/LogEntry.hpp"

#include <string>
#include <vector>
#include <sstream>

RealGameFacade::RealGameFacade() {
    vm.statusLine = "Nimonspoli siap. Mulai permainan baru atau muat savefile.";
    vm.footerHint = "New Game untuk memulai.";
}

const GameViewModel& RealGameFacade::getViewModel() const {
    return vm;
}

void RealGameFacade::tick(float) {
    // Tidak dibutuhkan di real game (game loop dikelola Game::run)
}

void RealGameFacade::startNewGame(const std::vector<std::string>& playerNames) {
    // Buat game baru via GameManager
    gameManager.startNewGame();
    Game* game = gameManager.getCurrentGame();
    if (!game) return;

    // Setup pemain
    std::vector<Player>& players = game->getPlayers();
    players.clear();
    int startMoney = game->getConfig().getMiscConfig(SALDO_AWAL);
    if (startMoney <= 0) startMoney = 1500;
    int maxTurn = game->getConfig().getMiscConfig(MAX_TURN);

    // Buat TurnManager dengan order pemain
    std::vector<int> order;
    for (int i = 0; i < static_cast<int>(playerNames.size()); ++i) {
        std::string name = playerNames[static_cast<size_t>(i)];
        if (name.empty()) name = "Player " + std::to_string(i + 1);
        // Buat Account temporary
        Account acc(name, "pass", 0);
        accountManager.addAccount(acc);
        Account* accPtr = accountManager.getAccount(name, "pass");
        players.emplace_back(i, accPtr, startMoney);
        order.push_back(i);
    }
    game->getTurnManager() = TurnManager(order, maxTurn);

    rebuildViewModel();
    vm.statusLine = "Permainan dimulai! Giliran: " + vm.players[0].name;
    vm.footerHint = "Klik Roll Dice untuk melempar dadu.";
}

void RealGameFacade::loadDemoGame() {
    // Demo: mulai game dengan 4 pemain default
    startNewGame({"Kebin", "Stewart", "Gro", "Bob"});
}

void RealGameFacade::selectTile(int index) {
    Game* game = gameManager.getCurrentGame();
    if (!game) return;
    int sz = game->getBoard().size();
    if (index >= 0 && index < sz) {
        vm.selectedTileIndex = index;
        // Tampilkan info tile di statusLine
        Tile* tile = game->getBoard().getTileByIndex(index);
        if (tile) {
            vm.statusLine = "Dipilih: [" + tile->getCode() + "] " + tile->getName();
        }
    }
}

void RealGameFacade::rollDice() {
    Game* game = gameManager.getCurrentGame();
    if (!game || game->isGameOver()) return;
    // Eksekusi satu giliran roll via Game internal
    // Memanggil satu langkah dari startTurn — kita modelling executeCommand
    game->executeCommand("LEMPAR_DADU");
    rebuildViewModel();
}

void RealGameFacade::advanceTurn() {
    Game* game = gameManager.getCurrentGame();
    if (!game || game->isGameOver()) return;
    game->executeCommand("AKHIRI_GILIRAN");
    rebuildViewModel();
}

void RealGameFacade::openSelectedTileDetails() {
    Game* game = gameManager.getCurrentGame();
    if (!game || vm.selectedTileIndex < 0) return;
    Tile* tile = game->getBoard().getTileByIndex(vm.selectedTileIndex);
    if (!tile) return;

    std::vector<std::string> lines = {
        "Kode: " + tile->getCode(),
        "Nama: " + tile->getName(),
    };

    PropertyTile* pt = dynamic_cast<PropertyTile*>(tile);
    if (pt) {
        lines.push_back("Harga: M" + std::to_string(pt->getLandPrice()));
        std::string ownerName = "BANK";
        if (pt->getOwner()) ownerName = pt->getOwner()->getUsername();
        lines.push_back("Pemilik: " + ownerName);
        lines.push_back("Status: " + std::string(pt->isMortgaged() ? "DIGADAIKAN" : pt->isOwned() ? "DIMILIKI" : "BANK"));

        StreetTile* st = dynamic_cast<StreetTile*>(pt);
        if (st) {
            lines.push_back("Grup warna: " + st->getColorGroup());
            lines.push_back("Rumah: " + std::to_string(st->getHouseCount()) + (st->hasHotelBuilt() ? " (Hotel)" : ""));
            if (pt->getFestivalMultiplier() > 1)
                lines.push_back("Festival x" + std::to_string(pt->getFestivalMultiplier())
                               + " (" + std::to_string(pt->getFestivalDuration()) + " giliran)");
        }
    }

    showOverlay(OverlayType::Info, tile->getName(), lines, "Klik X untuk menutup.");
}

void RealGameFacade::showCurrentPlayerProperties() {
    Game* game = gameManager.getCurrentGame();
    if (!game) return;
    int idx = game->getTurnManager().getCurrentPlayerIndex();
    if (idx < 0 || idx >= static_cast<int>(game->getPlayers().size())) return;
    Player& p = game->getPlayer(idx);

    std::vector<std::string> lines = {"Pemilik: " + p.getUsername()};
    for (PropertyTile* pt : p.getOwnedProperties()) {
        std::string entry = pt->getCode() + " - " + pt->getName();
        if (pt->isMortgaged()) entry += " [GADAI]";
        lines.push_back(entry);
    }
    if (lines.size() == 1) lines.push_back("(tidak ada properti)");

    showOverlay(OverlayType::Properties, "Properti " + p.getUsername(), lines, "");
}

void RealGameFacade::showCardPopup() {
    Game* game = gameManager.getCurrentGame();
    if (!game) return;
    int idx = game->getTurnManager().getCurrentPlayerIndex();
    if (idx < 0) return;
    Player& p = game->getPlayer(idx);

    std::vector<std::string> lines;
    for (const auto& card : p.getHandCards()) {
        lines.push_back(card->getName() + " - " + card->getDescription());
    }
    if (lines.empty()) lines.push_back("(tidak ada kartu kemampuan)");

    showOverlay(OverlayType::Cards, "Kartu Kemampuan", lines,
                "Gunakan perintah GUNAKAN_KEMAMPUAN <index>.");
}

void RealGameFacade::showFestivalPopup() {
    showOverlay(OverlayType::Festival, "Festival",
                {"Landilah petak Festival untuk mengaktifkan.",
                 "Sewa properti pilihan berlipat ganda selama 3 giliran.",
                 "Maksimum 3 kali penggandaan (max 8x)."},
                "");
}

void RealGameFacade::showTaxPopup() {
    Game* game = gameManager.getCurrentGame();
    if (!game) return;
    int ppfFlat = game->getConfig().getTaxConfig(PPH);
    int pbmFlat = game->getConfig().getTaxConfig(PBM);
    showOverlay(OverlayType::Tax, "Pajak",
                {"PPH: M" + std::to_string(ppfFlat) + " flat ATAU 10% dari total kekayaan.",
                 "Pilih sebelum menghitung kekayaan!",
                 "PBM: M" + std::to_string(pbmFlat) + " flat langsung."},
                "");
}

void RealGameFacade::showAuctionPopup() {
    Game* game = gameManager.getCurrentGame();
    if (!game) return;
    if (!game->getAuctionManager().isAuctionActive()) {
        showOverlay(OverlayType::Auction, "Lelang",
                    {"Tidak ada lelang aktif.", "Pilih LELANG saat berada di properti BANK."},
                    "");
        return;
    }
    Player* bidder = game->getAuctionManager().getCurrentTurnPlayer();
    showOverlay(OverlayType::Auction, "Lelang Aktif",
                {"Giliran: " + (bidder ? bidder->getUsername() : "?"),
                 "Bid tertinggi: M" + std::to_string(game->getAuctionManager().getHighestBid()),
                 "Gunakan TAWAR <jumlah> atau PASS."},
                "");
}

void RealGameFacade::showSavePopup() {
    showOverlay(OverlayType::SaveLoad, "Simpan / Muat",
                {"Gunakan perintah SIMPAN <nama_file> untuk menyimpan.",
                 "Muat hanya bisa sebelum game dimulai.",
                 "Format file: data/<nama_file>.txt"},
                "");
}

void RealGameFacade::showVictoryPopup() {
    Game* game = gameManager.getCurrentGame();
    if (!game) return;

    std::vector<std::string> lines;
    // Sort players by wealth (descending)
    std::vector<Player*> sorted;
    for (Player& p : game->getPlayers()) sorted.push_back(&p);
    std::sort(sorted.begin(), sorted.end(),
              [](Player* a, Player* b){ return a->getTotalWealth() > b->getTotalWealth(); });

    lines.push_back("=== Klasemen Akhir ===");
    for (size_t i = 0; i < sorted.size(); ++i) {
        lines.push_back(std::to_string(i+1) + ". " + sorted[i]->getUsername()
                      + " - M" + std::to_string(sorted[i]->getTotalWealth())
                      + (sorted[i]->isBankrupt() ? " [BANGKRUT]" : ""));
    }

    showOverlay(OverlayType::Victory, "Permainan Selesai!", lines, "Terima kasih telah bermain Nimonspoli!");
}

void RealGameFacade::closeOverlay() {
    vm.overlay = OverlayViewData{};
}

// ── Private helpers ──────────────────────────────────────────

void RealGameFacade::showOverlay(OverlayType type, const std::string& title,
                                  const std::vector<std::string>& lines,
                                  const std::string& footer) {
    vm.overlay.type = type;
    vm.overlay.title = title;
    vm.overlay.lines = lines;
    vm.overlay.footer = footer;
}

TileKind RealGameFacade::tileKindFromCode(const std::string& code,
                                           const std::string& colorGroup) const {
    if (!colorGroup.empty() && colorGroup != "DEFAULT" && colorGroup != "ABU_ABU") {
        return TileKind::Street;
    }
    if (colorGroup == "ABU_ABU") return TileKind::Utility;
    // heuristic dari kode
    if (code == "ST1" || code == "ST2" || code == "ST3" || code == "ST4") return TileKind::Railroad;
    if (code == "PLN" || code == "PAM") return TileKind::Utility;
    if (code == "KES1" || code == "KES2") return TileKind::Card;
    if (code == "DU1" || code == "DU2") return TileKind::Card;
    if (code == "FEST1" || code == "FEST2") return TileKind::Festival;
    if (code == "PPH" || code == "PBM") return TileKind::Tax;
    return TileKind::Special;
}

void RealGameFacade::rebuildViewModel() {
    Game* game = gameManager.getCurrentGame();
    if (!game) return;

    // Board tiles
    vm.board.clear();
    for (Tile* tile : game->getBoard().getTiles()) {
        if (!tile) continue;
        TileViewData td;
        td.code = tile->getCode();
        td.name = tile->getName();
        td.ownerIndex = -1;
        td.status = PropertyStatusView::Bank;
        td.houses = 0;
        td.hotel = false;
        td.festivalMultiplier = 1;

        PropertyTile* pt = dynamic_cast<PropertyTile*>(tile);
        if (pt) {
            if (pt->getOwner()) {
                // find owner index
                int ownerIdx = 0;
                for (const Player& p : game->getPlayers()) {
                    if (&p == pt->getOwner()) { td.ownerIndex = ownerIdx; break; }
                    ownerIdx++;
                }
            }
            switch (pt->getStatus()) {
                case BANK:      td.status = PropertyStatusView::Bank; break;
                case OWNED:     td.status = PropertyStatusView::Owned; break;
                case MORTGAGED: td.status = PropertyStatusView::Mortgaged; break;
            }
            td.price = pt->getLandPrice();
            td.festivalMultiplier = pt->getFestivalMultiplier();

            StreetTile* st = dynamic_cast<StreetTile*>(pt);
            if (st) {
                td.colorGroup = st->getColorGroup();
                td.houses = st->getHouseCount();
                td.hotel = st->hasHotelBuilt();
                td.kind = TileKind::Street;
            } else if (dynamic_cast<RailroadTile*>(pt)) {
                td.kind = TileKind::Railroad;
                td.colorGroup = "DEFAULT";
            } else if (dynamic_cast<UtilityTile*>(pt)) {
                td.kind = TileKind::Utility;
                td.colorGroup = "ABU_ABU";
            }
        } else {
            td.kind = tileKindFromCode(td.code, "");
            td.colorGroup = "DEFAULT";
        }

        vm.board.push_back(td);
    }

    // Players
    vm.players.clear();
    int currentIdx = game->getTurnManager().getCurrentPlayerIndex();
    int i = 0;
    for (const Player& p : game->getPlayers()) {
        PlayerViewData pd;
        pd.name = p.getUsername();
        pd.money = p.getMoney();
        pd.position = p.getPosition();
        pd.isCurrent = (i == currentIdx);
        switch (p.getStatus()) {
            case PlayerStatus::ACTIVE:   pd.status = "ACTIVE"; break;
            case PlayerStatus::JAILED:   pd.status = "JAILED"; break;
            case PlayerStatus::BANKRUPT: pd.status = "BANKRUPT"; break;
        }
        vm.players.push_back(pd);
        i++;
    }

    // Hand cards of current player
    vm.hand.clear();
    if (currentIdx >= 0 && currentIdx < static_cast<int>(game->getPlayers().size())) {
        for (const auto& card : game->getPlayer(currentIdx).getHandCards()) {
            CardViewData cd;
            cd.title = card->getName();
            cd.description = card->getDescription();
            vm.hand.push_back(cd);
        }
    }

    // Logs
    vm.logs.clear();
    for (const LogEntry& entry : game->getLogger().getEntries()) {
        LogEntryViewData ld;
        ld.turn   = entry.getTurn();
        ld.actor  = entry.getUsername();
        ld.type   = entry.getActionType();
        ld.detail = entry.getDetail();
        vm.logs.push_back(ld);
    }

    vm.currentPlayerIndex = currentIdx >= 0 ? currentIdx : 0;
    vm.currentTurn = game->getTurnManager().getCurrentTurn();
    vm.maxTurn = game->getTurnManager().getMaxTurn();
    vm.selectedTileIndex = std::max(0, std::min(vm.selectedTileIndex, static_cast<int>(vm.board.size()) - 1));

    if (game->isGameOver()) {
        vm.statusLine = "Permainan selesai!";
    } else if (!vm.players.empty()) {
        vm.statusLine = "Giliran: " + vm.players[static_cast<size_t>(vm.currentPlayerIndex)].name;
    }
}
