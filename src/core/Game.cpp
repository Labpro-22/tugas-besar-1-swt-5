#include "../../include/core/Game.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/core/BankruptException.hpp"
#include "../../include/core/InvalidActionException.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

Game::Game()
    : board(),
      players(),
      turnManager(vector<int>{}, 0),
      dice(),
      cardManager(),
      auctionManager(),
      bankruptcyManager(&liquidationManager),
      tradeManager(),
      liquidationManager(),
      logger(),
      config(),
      gameOver(false) {}

// ──────────────────────────────────────────────
// GAME LOOP
// ──────────────────────────────────────────────

void Game::run() {
    while (!gameOver) {
        startTurn();
    }
}

void Game::startTurn() {
    if (players.empty() || gameOver) return;

    int playerIdx = turnManager.getCurrentPlayerIndex();
    Player& current = players[static_cast<size_t>(playerIdx)];

    // Reset flag awal giliran
    current.resetTurnFlags();

    // Beri kartu kemampuan ke SEMUA pemain aktif
    for (Player& p : players) {
        if (!p.isBankrupt()) {
            cardManager.giveTurnStartAbility(&p);
            // Cek overflow: max 3 kartu
            while (p.getHandCards().size() > 3) {
                cout << "[" << p.getUsername() << "] Tangan penuh! Pilih kartu yang dibuang (0-"
                     << p.getHandCards().size() - 1 << "): ";
                int dropIdx = 0;
                cin >> dropIdx;
                cin.ignore();
                try {
                    auto dropped = p.dropCard(dropIdx);
                    cardManager.discardAbilityCard(move(dropped));
                } catch (...) {
                    p.dropCard(static_cast<int>(p.getHandCards().size()) - 1);
                }
            }
        }
    }

    // Tampilkan papan
    map<int, vector<string>> markers;
    for (const Player& p : players) {
        if (!p.isBankrupt()) markers[p.getPosition()].push_back(p.getUsername());
    }
    string turnInfo = "Turn " + to_string(turnManager.getCurrentTurn())
                    + " | " + current.getUsername()
                    + " | M" + to_string(current.getMoney());
    board.printBoard(markers, turnInfo);

    // REPL
    bool turnEnded = false;
    bool hasRolled = false;

    while (!turnEnded && !gameOver) {
        cout << "\n[" << current.getUsername() << "] >> ";
        string line;
        if (!getline(cin, line)) { gameOver = true; break; }

        istringstream ss(line);
        string cmd;
        ss >> cmd;
        for (char& c : cmd) c = toupper(c);

        try {

            if (cmd == "CETAK_PAPAN") {
                board.printBoard(markers, turnInfo);

            } else if (cmd == "CETAK_PROPERTI") {
                string who;
                ss >> who;
                Player* target = &current;
                if (!who.empty()) {
                    for (Player& p : players) {
                        if (p.getUsername() == who) { target = &p; break; }
                    }
                }
                if (target->getOwnedProperties().empty()) {
                    cout << target->getUsername() << " tidak memiliki properti.\n";
                } else {
                    for (PropertyTile* pt : target->getOwnedProperties())
                        cout << pt->toString() << "\n";
                }

            } else if (cmd == "CETAK_AKTA") {
                string code; ss >> code;
                PropertyTile* pt = dynamic_cast<PropertyTile*>(board.getTileByCode(code));
                if (!pt) cout << "Bukan petak properti.\n";
                else cout << pt->toString() << "\n";

            } else if (cmd == "CETAK_LOG") {
                logger.printLog();

            } else if (cmd == "CETAK_KARTU") {
                const auto& hand = current.getHandCards();
                if (hand.empty()) cout << "Tidak memiliki kartu kemampuan.\n";
                else for (size_t i = 0; i < hand.size(); ++i)
                    cout << i << ". " << hand[i]->getName()
                         << " - " << hand[i]->getDescription() << "\n";

            } else if (cmd == "ATUR_DADU") {
                if (hasRolled) { cout << "Dadu sudah dilempar.\n"; continue; }
                int d1 = 0, d2 = 0; ss >> d1 >> d2;
                if (d1 < 1 || d1 > 6 || d2 < 1 || d2 > 6)
                    cout << "Nilai dadu harus 1-6.\n";
                else { dice.setManual(d1, d2); cout << "Dadu diatur: " << d1 << "+" << d2 << "\n"; }

            } else if (cmd == "GUNAKAN_KEMAMPUAN") {
                if (hasRolled) { cout << "Hanya bisa sebelum lempar dadu.\n"; continue; }
                if (current.hasUsedAbilityThisTurn()) { cout << "Sudah pakai kartu giliran ini.\n"; continue; }
                int idx = 0; ss >> idx;
                current.useAbilityCard(idx, this);
                logger.log(turnManager.getCurrentTurn(), current.getUsername(),
                           "KARTU_KEMAMPUAN", "Kartu ke-" + to_string(idx));

            } else if (cmd == "LEMPAR_DADU") {
                if (hasRolled) { cout << "Dadu sudah dilempar.\n"; continue; }

                auto result = dice.roll();
                int d1 = result.first, d2 = result.second;
                cout << current.getUsername() << " melempar: " << d1 << "+" << d2 << "=" << d1+d2 << "\n";

                if (current.getStatus() == PlayerStatus::JAILED) {
                    // Di penjara
                    if (dice.isDouble(result)) {
                        current.releaseFromJail();
                        cout << "Double! Keluar penjara.\n";
                        logger.log(turnManager.getCurrentTurn(), current.getUsername(), "PENJARA", "Keluar dengan double");
                        bool passedGo = false;
                        int newPos = board.calculateNewPosition(current.getPosition(), d1+d2, passedGo);
                        current.moveTo(newPos);
                        if (passedGo) {
                            int salary = config.getSpecialConfig(GO_SALARY);
                            current.receive(salary);
                            logger.log(turnManager.getCurrentTurn(), current.getUsername(), "GO", "M" + to_string(salary));
                        }
                        handleLanding(current);
                    } else {
                        current.incrementJailAttempt();
                        cout << "Tidak double. Percobaan " << current.getJailTurnsAttempted() << "/3.\n";
                        if (current.getJailTurnsAttempted() >= 3) {
                            int fine = config.getSpecialConfig(JAIL_FINE);
                            cout << "Giliran ke-4! Wajib bayar denda M" << fine << ".\n";
                            current.pay(fine);
                            current.releaseFromJail();
                            logger.log(turnManager.getCurrentTurn(), current.getUsername(), "PENJARA", "Bayar denda M" + to_string(fine));
                        }
                        hasRolled = true;
                        turnEnded = true;
                        endTurn();
                        return;
                    }
                } else {
                    // Normal
                    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "DADU",
                               to_string(d1) + "+" + to_string(d2));
                    bool isDouble = dice.isDouble(result);
                    if (isDouble) {
                        current.consecutiveDoubleCount++;
                        if (current.getConsecutiveDoubleCount() >= 3) {
                            cout << "3 double! Masuk penjara.\n";
                            current.moveTo(board.getJailIndex());
                            current.enterJail();
                            logger.log(turnManager.getCurrentTurn(), current.getUsername(), "PENJARA", "3 double berturut");
                            hasRolled = true; turnEnded = true;
                            endTurn(); return;
                        }
                    }
                    bool passedGo = false;
                    int newPos = board.calculateNewPosition(current.getPosition(), d1+d2, passedGo);
                    current.moveTo(newPos);
                    if (passedGo) {
                        int salary = config.getSpecialConfig(GO_SALARY);
                        current.receive(salary);
                        cout << "Melewati GO! Terima M" << salary << ".\n";
                        logger.log(turnManager.getCurrentTurn(), current.getUsername(), "GO", "M" + to_string(salary));
                    }
                    handleLanding(current);

                    if (isDouble && current.getStatus() != PlayerStatus::JAILED) {
                        cout << "Double! Giliran ekstra.\n";
                        turnManager.grantExtraTurn();
                        hasRolled = true; turnEnded = true;
                        endTurn(); return;
                    }
                }
                hasRolled = true;

            } else if (cmd == "BELI") {
                if (!hasRolled) { cout << "Lempar dadu dulu.\n"; continue; }
                PropertyTile* pt = dynamic_cast<PropertyTile*>(board.getTileByIndex(current.getPosition()));
                if (!pt || pt->getStatus() != BANK) { cout << "Tidak ada properti BANK di sini.\n"; continue; }
                int price = pt->getLandPrice();
                if (!current.canAfford(price)) { cout << "Uang tidak cukup. Perlu M" << price << ".\n"; continue; }
                current.pay(price);
                pt->setOwner(&current);
                current.addProperty(pt);
                cout << "Beli " << pt->getName() << " M" << price << ".\n";
                logger.log(turnManager.getCurrentTurn(), current.getUsername(), "BELI", pt->getName() + " M" + to_string(price));

            } else if (cmd == "LELANG") {
                if (!hasRolled) { cout << "Lempar dadu dulu.\n"; continue; }
                PropertyTile* pt = dynamic_cast<PropertyTile*>(board.getTileByIndex(current.getPosition()));
                if (!pt || pt->getStatus() != BANK) { cout << "Tidak ada properti untuk dilelang.\n"; continue; }
                vector<Player*> bidders;
                for (Player& p : players) { if (!p.isBankrupt()) bidders.push_back(&p); }
                auctionManager.runAuction(*pt, bidders, current, *this);
                cout << "Lelang dimulai. Gunakan TAWAR <jumlah> atau PASS.\n";

            } else if (cmd == "TAWAR") {
                if (!auctionManager.isAuctionActive()) { cout << "Tidak ada lelang aktif.\n"; continue; }
                int amount = 0; ss >> amount;
                auctionManager.processAction("BID", amount);

            } else if (cmd == "PASS") {
                if (!auctionManager.isAuctionActive()) { cout << "Tidak ada lelang aktif.\n"; continue; }
                auctionManager.processAction("PASS");

            } else if (cmd == "BANGUN") {
                string code; ss >> code;
                StreetTile* st = dynamic_cast<StreetTile*>(board.getTileByCode(code));
                if (!st || st->getOwner() != &current) { cout << "Bukan StreetTile milik Anda.\n"; continue; }
                if (st->getBuildingLevel() < 4 && st->canBuildHouse(this)) {
                    int cost = st->getHouseBuildCost();
                    if (!current.canAfford(cost)) { cout << "Perlu M" << cost << ".\n"; continue; }
                    current.pay(cost); st->buildHouse();
                    cout << "Rumah dibangun di " << st->getName() << ". Total: " << st->getHouseCount() << "\n";
                    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "BANGUN", "Rumah di " + st->getName());
                } else if (st->getBuildingLevel() == 4 && st->canBuildHotel(this)) {
                    int cost = st->getHotelBuildCost();
                    if (!current.canAfford(cost)) { cout << "Perlu M" << cost << ".\n"; continue; }
                    current.pay(cost); st->buildHotel();
                    cout << "Hotel dibangun di " << st->getName() << ".\n";
                    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "BANGUN", "Hotel di " + st->getName());
                } else { cout << "Tidak bisa bangun di " << code << ".\n"; }

            } else if (cmd == "GADAI") {
                string code; ss >> code;
                PropertyTile* pt = dynamic_cast<PropertyTile*>(board.getTileByCode(code));
                if (!pt || pt->getOwner() != &current) { cout << "Bukan properti Anda.\n"; continue; }
                if (pt->isMortgaged()) { cout << "Sudah digadaikan.\n"; continue; }
                if (!pt->canBeMortgaged(this)) { cout << "Tidak bisa digadaikan (ada bangunan di colorgroup).\n"; continue; }
                int val = pt->mortgage(); current.receive(val);
                cout << pt->getName() << " digadaikan. Terima M" << val << ".\n";
                logger.log(turnManager.getCurrentTurn(), current.getUsername(), "GADAI", pt->getName() + " M" + to_string(val));

            } else if (cmd == "TEBUS") {
                string code; ss >> code;
                PropertyTile* pt = dynamic_cast<PropertyTile*>(board.getTileByCode(code));
                if (!pt || pt->getOwner() != &current) { cout << "Bukan properti Anda.\n"; continue; }
                if (!pt->isMortgaged()) { cout << "Tidak sedang digadaikan.\n"; continue; }
                int price = pt->getLandPrice();
                if (!current.canAfford(price)) { cout << "Perlu M" << price << ".\n"; continue; }
                current.pay(price); pt->redeem();
                cout << pt->getName() << " ditebus M" << price << ".\n";
                logger.log(turnManager.getCurrentTurn(), current.getUsername(), "TEBUS", pt->getName() + " M" + to_string(price));

            } else if (cmd == "KELUAR_PENJARA_BAYAR") {
                if (current.getStatus() != PlayerStatus::JAILED) { cout << "Tidak di penjara.\n"; continue; }
                if (hasRolled) { cout << "Sudah terlambat.\n"; continue; }
                int fine = config.getSpecialConfig(JAIL_FINE);
                if (!current.canAfford(fine)) { cout << "Tidak cukup M" << fine << ".\n"; continue; }
                current.pay(fine); current.releaseFromJail();
                cout << "Keluar penjara bayar M" << fine << ".\n";
                logger.log(turnManager.getCurrentTurn(), current.getUsername(), "PENJARA", "Bayar denda M" + to_string(fine));

            } else if (cmd == "SIMPAN") {
                if (hasRolled) { cout << "Simpan hanya bisa sebelum lempar dadu.\n"; continue; }
                cout << "(Simpan ditangani GameManager)\n";

            } else if (cmd == "AKHIRI_GILIRAN") {
                if (!hasRolled) { cout << "Lempar dadu dulu.\n"; continue; }
                turnEnded = true;

            } else if (!cmd.empty()) {
                cout << "Perintah tidak dikenal: " << cmd << "\n";
            }

        } catch (BankruptException& e) {
            cout << "[BANGKRUT] " << e.getMessage() << "\n";
            current.setStatus(PlayerStatus::BANKRUPT);
            turnManager.removePlayer(playerIdx);
            turnEnded = true;
        } catch (InvalidActionException& e) {
            cout << "[AKSI TIDAK VALID] " << e.getMessage() << "\n";
        } catch (exception& e) {
            cout << "[ERROR] " << e.what() << "\n";
        }

        checkWinCondition();
        if (gameOver) break;
    }

    if (!gameOver) endTurn();
}

void Game::endTurn() {
    if (players.empty()) return;
    int playerIdx = turnManager.getCurrentPlayerIndex();
    Player& current = players[static_cast<size_t>(playerIdx)];

    // Decrement festival semua properti milik pemain aktif
    for (PropertyTile* pt : current.getOwnedProperties()) {
        pt->decrementFestivalDuration();
        pt->resetFestivalIfExpired();
    }

    current.consecutiveDoubleCount = 0;
    turnManager.nextPlayer();
    checkWinCondition();
}

void Game::handleLanding(Player& player) {
    Tile* tile = board.getTileByIndex(player.getPosition());
    if (tile != nullptr) tile->onLand(&player, this);
}

void Game::checkWinCondition() {
    int active = 0;
    for (const Player& p : players) {
        if (!p.isBankrupt()) active++;
    }

    if (active <= 1) {
        gameOver = true;
        cout << "\n=== PERMAINAN SELESAI ===\n";
        for (const Player& p : players) {
            if (!p.isBankrupt()) {
                cout << "Pemenang: " << p.getUsername() << "!\n";
                break;
            }
        }
        return;
    }

    if (turnManager.isMaxTurnReached()) {
        gameOver = true;
        cout << "\n=== BATAS TURN TERCAPAI ===\n";
        Player* winner = nullptr;
        for (Player& p : players) {
            if (!p.isBankrupt() && (winner == nullptr || p > *winner))
                winner = &p;
        }
        if (winner)
            cout << "Pemenang: " << winner->getUsername()
                 << " (M" << winner->getTotalWealth() << ")\n";
    }
}

void Game::executeCommand(const std::string& command) {
    (void)command;
}

// Getters
Board& Game::getBoard() { return board; }
vector<Player>& Game::getPlayers() { return players; }
Player& Game::getPlayer(int index) { return players.at(static_cast<size_t>(index)); }
TurnManager& Game::getTurnManager() { return turnManager; }
Dice& Game::getDice() { return dice; }
CardManager& Game::getCardManager() { return cardManager; }
AuctionManager& Game::getAuctionManager() { return auctionManager; }
BankruptcyManager& Game::getBankruptcyManager() { return bankruptcyManager; }
TradeManager& Game::getTradeManager() { return tradeManager; }
LiquidationManager& Game::getLiquidationManager() { return liquidationManager; }
Logger& Game::getLogger() { return logger; }
Config& Game::getConfig() { return config; }
void Game::setConfig(const Config& cfg) { config = cfg; }
bool Game::isGameOver() const { return gameOver; }
void Game::setGameOver(bool status) { gameOver = status; }
void Game::giveSalary(Player& player) {
    player.receive(config.getSpecialConfig(0));
}
