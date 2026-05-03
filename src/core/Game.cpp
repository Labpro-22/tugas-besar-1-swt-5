#include "../../include/core/Game.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/models/DerivedAbilityCard.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/core/BankruptException.hpp"
#include "../../include/core/InvalidActionException.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace {
PropertyTile* currentBankProperty(Game& game, Player& player) {
    if (game.getBoard().size() == 0) {
        return nullptr;
    }

    PropertyTile* property = dynamic_cast<PropertyTile*>(
        game.getBoard().getTileByIndex(player.getPosition())
    );

    if (property == nullptr || property->getStatus() != BANK) {
        return nullptr;
    }

    return property;
}
}

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
      gameOver(false),
      lastDiceTotal(0),
      hasRolledThisTurn(false),
      extraRollPending(false) {}

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
    hasRolledThisTurn = false;

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
    PropertyTile* resolvedPropertyDecision = nullptr;
    extraRollPending = false;

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
                if (hasRolled && !extraRollPending) { cout << "Dadu sudah dilempar.\n"; continue; }
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
                if (hasRolled && !extraRollPending) { cout << "Dadu sudah dilempar.\n"; continue; }
                if (extraRollPending) {
                    PropertyTile* blockingProperty = currentBankProperty(*this, current);
                    if (blockingProperty != nullptr && blockingProperty != resolvedPropertyDecision) {
                        cout << "Selesaikan BELI atau LELANG " << blockingProperty->getName()
                             << " sebelum lempar dadu lagi.\n";
                        continue;
                    }
                    if (auctionManager.isAuctionActive()) {
                        cout << "Selesaikan lelang aktif sebelum lempar dadu lagi.\n";
                        continue;
                    }
                    hasRolled = false;
                    hasRolledThisTurn = false;
                    extraRollPending = false;
                }
                if (current.getStatus() == PlayerStatus::JAILED && current.getJailTurnsAttempted() >= 3) {
                    cout << "Sudah gagal 3 kali. Giliran ini wajib bayar denda penjara sebelum lempar dadu.\n";
                    continue;
                }

                auto result = dice.roll();
                int d1 = result.first, d2 = result.second;
                setLastDiceTotal(d1 + d2);
                hasRolledThisTurn = true;
                resolvedPropertyDecision = nullptr;
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
                            awardGoSalary(current);
                        }
                        handleLanding(current);
                    } else {
                        current.incrementJailAttempt();
                        cout << "Tidak double. Percobaan " << current.getJailTurnsAttempted() << "/3.\n";
                        if (current.getJailTurnsAttempted() >= 3) {
                            int fine = config.getSpecialConfig(JAIL_FINE);
                            cout << "Batas percobaan habis. Pada giliran penjara berikutnya wajib bayar denda M" << fine << ".\n";
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
                        current.addConsecutiveDouble();
                        if (current.getConsecutiveDoubleCount() >= 3) {
                            cout << "3 double! Masuk penjara.\n";
                            int jailIndex = board.getJailIndex();
                            if (jailIndex >= 0) current.moveTo(jailIndex);
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
                        awardGoSalary(current);
                        cout << "Melewati GO! Terima gaji.\n";
                    }
                    handleLanding(current);

                    if (isDouble && current.getStatus() != PlayerStatus::JAILED) {
                        cout << "Double! Selesaikan BELI/LELANG jika ada, lalu lempar dadu lagi.\n";
                        extraRollPending = true;
                    } else if (!isDouble) {
                        current.resetConsecutiveDouble();
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
                resolvedPropertyDecision = pt;
                cout << "Beli " << pt->getName() << " M" << price << ".\n";
                logger.log(turnManager.getCurrentTurn(), current.getUsername(), "BELI", pt->getName() + " M" + to_string(price));

            } else if (cmd == "LELANG") {
                if (!hasRolled) { cout << "Lempar dadu dulu.\n"; continue; }
                PropertyTile* pt = dynamic_cast<PropertyTile*>(board.getTileByIndex(current.getPosition()));
                if (!pt || pt->getStatus() != BANK) { cout << "Tidak ada properti untuk dilelang.\n"; continue; }
                vector<Player*> bidders;
                for (Player& p : players) { if (!p.isBankrupt()) bidders.push_back(&p); }
                auctionManager.runAuction(*pt, bidders, current, *this);
                resolvedPropertyDecision = pt;
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
                payBankOrBankrupt(current, fine, "Denda penjara");
                if (!current.isBankrupt()) current.releaseFromJail();
                cout << "Keluar penjara bayar M" << fine << ".\n";
                logger.log(turnManager.getCurrentTurn(), current.getUsername(), "PENJARA", "Bayar denda M" + to_string(fine));

            } else if (cmd == "KELUAR_PENJARA_KARTU") {
                if (current.getStatus() != PlayerStatus::JAILED) { cout << "Tidak di penjara.\n"; continue; }
                if (hasRolled) { cout << "Sudah terlambat.\n"; continue; }
                if (useJailFreeCardForCurrentPlayer()) {
                    cout << "Keluar penjara dengan kartu Bebas dari Penjara.\n";
                } else {
                    cout << "Tidak memiliki kartu Bebas dari Penjara.\n";
                }

            } else if (cmd == "SIMPAN") {
                if (hasRolled) { cout << "Simpan hanya bisa sebelum lempar dadu.\n"; continue; }
                cout << "(Simpan ditangani GameManager)\n";

            } else if (cmd == "AKHIRI_GILIRAN") {
                if (!hasRolled) { cout << "Lempar dadu dulu.\n"; continue; }
                if (auctionManager.isAuctionActive()) { cout << "Selesaikan lelang aktif dulu.\n"; continue; }
                PropertyTile* blockingProperty = currentBankProperty(*this, current);
                if (blockingProperty != nullptr && blockingProperty != resolvedPropertyDecision) {
                    cout << "Properti " << blockingProperty->getName()
                         << " belum diproses. Pilih BELI atau LELANG dulu.\n";
                    continue;
                }
                if (extraRollPending) {
                    cout << "Masih ada giliran ekstra karena double. Lempar dadu lagi.\n";
                    continue;
                }
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
    if (players.empty() || gameOver) {
        return;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();

    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];

    // Decrement festival semua properti milik pemain aktif
    for (PropertyTile* pt : current.getOwnedProperties()) {
        if (pt != nullptr) {
            pt->decrementFestivalDuration();
            pt->resetFestivalIfExpired();
        }
    }

    current.consecutiveDoubleCount = 0;
    hasRolledThisTurn = false;
    extraRollPending = false;

    const bool finalTurnFinished =
        turnManager.getMaxTurn() > 0 &&
        turnManager.getCurrentTurn() >= turnManager.getMaxTurn() &&
        turnManager.isLastPlayerInCurrentTurn();

    if (finalTurnFinished) {
        gameOver = true;

        Player* winner = nullptr;

        for (Player& p : players) {
            if (!p.isBankrupt() &&
                (winner == nullptr || p.getTotalWealth() > winner->getTotalWealth())) {
                winner = &p;
            }
        }

        std::string detail = "Batas maksimum turn tercapai.";

        if (winner != nullptr) {
            detail += " Pemenang: " + winner->getUsername() +
                      " dengan total kekayaan M" + std::to_string(winner->getTotalWealth());
        }

        logger.log(
            turnManager.getCurrentTurn(),
            "System",
            "GAME_OVER",
            detail
        );

        cout << "\n=== BATAS TURN TERCAPAI ===\n";

        if (winner != nullptr) {
            cout << "Pemenang: " << winner->getUsername()
                 << " (M" << winner->getTotalWealth() << ")\n";
        }

        return;
    }

    turnManager.nextPlayer();
    checkWinCondition();
}

void Game::handleLanding(Player& player) {
    if (board.size() == 0) {
        logger.log(turnManager.getCurrentTurn(), player.getUsername(), "BOARD", "Board kosong, landing dilewati");
        return;
    }
    Tile* tile = board.getTileByIndex(player.getPosition());
    if (tile != nullptr) tile->onLand(&player, this);
}

std::pair<int, int> Game::rollDiceForCurrentPlayer() {
    if (players.empty() || gameOver || board.size() == 0 || hasRolledThisTurn) {
        return {0, 0};
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return {0, 0};
    }

    Player& current = players[static_cast<size_t>(playerIdx)];

    if (current.getStatus() == PlayerStatus::JAILED && current.getJailTurnsAttempted() >= 3) {
        logger.log(turnManager.getCurrentTurn(), current.getUsername(), "PENJARA",
                   "Wajib bayar denda sebelum lempar dadu");
        return {0, 0};
    }

    auto result = dice.roll();
    int diceTotal = result.first + result.second;
    const bool isDouble = dice.isDouble(result);
    setLastDiceTotal(diceTotal);
    hasRolledThisTurn = true;
    extraRollPending = false;
    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "DADU",
               to_string(result.first) + "+" + to_string(result.second) + "=" + to_string(diceTotal));

    if (current.getStatus() == PlayerStatus::JAILED) {
        if (isDouble) {
            current.releaseFromJail();
            logger.log(turnManager.getCurrentTurn(), current.getUsername(), "PENJARA", "Keluar dengan double");
        } else {
            current.incrementJailAttempt();
            logger.log(turnManager.getCurrentTurn(), current.getUsername(), "PENJARA",
                       "Gagal double percobaan " + to_string(current.getJailTurnsAttempted()) + "/3");
            return result;
        }
    } else if (isDouble) {
        current.addConsecutiveDouble();
        if (current.getConsecutiveDoubleCount() >= 3) {
            int jailIndex = board.getJailIndex();
            if (jailIndex >= 0) {
                current.moveTo(jailIndex);
            }
            current.enterJail();
            logger.log(turnManager.getCurrentTurn(), current.getUsername(), "PENJARA",
                       "3 double berturut");
            return result;
        }
    } else {
        current.resetConsecutiveDouble();
    }

    bool passedGo = false;
    int newPos = board.calculateNewPosition(current.getPosition(), diceTotal, passedGo);
    current.moveTo(newPos);
    if (passedGo) {
        awardGoSalary(current);
    }
    handleLanding(current);

    if (isDouble && current.getStatus() == PlayerStatus::ACTIVE) {
        extraRollPending = true;
        logger.log(turnManager.getCurrentTurn(), current.getUsername(), "DADU",
                   "Double, mendapat giliran ekstra");
    }

    return result;
}

void Game::endCurrentTurn() {
    endTurn();
}

bool Game::buyCurrentProperty() {
    if (players.empty() || board.size() == 0) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    PropertyTile* pt = dynamic_cast<PropertyTile*>(board.getTileByIndex(current.getPosition()));
    if (pt == nullptr || pt->getStatus() != BANK) {
        return false;
    }

    int price = pt->getLandPrice();
    if (!current.canAfford(price)) {
        logger.log(turnManager.getCurrentTurn(), current.getUsername(), "BELI_GAGAL",
                   pt->getName() + " perlu M" + to_string(price));
        return false;
    }

    current.pay(price);
    pt->setOwner(&current);
    current.addProperty(pt);
    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "BELI",
               pt->getName() + " M" + to_string(price));
    return true;
}

bool Game::mortgageProperty(const std::string& code) {
    if (players.empty() || board.size() == 0) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    PropertyTile* pt = dynamic_cast<PropertyTile*>(board.getTileByCode(code));
    if (pt == nullptr || pt->getOwner() != &current || pt->isMortgaged() || !pt->canBeMortgaged(this)) {
        return false;
    }

    int value = pt->mortgage();
    current.receive(value);
    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "GADAI",
               pt->getName() + " M" + to_string(value));
    return true;
}

bool Game::redeemProperty(const std::string& code) {
    if (players.empty() || board.size() == 0) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    PropertyTile* pt = dynamic_cast<PropertyTile*>(board.getTileByCode(code));
    if (pt == nullptr || pt->getOwner() != &current || !pt->isMortgaged()) {
        return false;
    }

    int price = pt->getLandPrice();
    if (!current.canAfford(price)) {
        return false;
    }

    current.pay(price);
    pt->redeem();
    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "TEBUS",
               pt->getName() + " M" + to_string(price));
    return true;
}

bool Game::buildProperty(const std::string& code) {
    if (players.empty() || board.size() == 0) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    StreetTile* st = dynamic_cast<StreetTile*>(board.getTileByCode(code));
    if (st == nullptr || st->getOwner() != &current) {
        return false;
    }

    if (st->getBuildingLevel() < 4 && st->canBuildHouse(this)) {
        int cost = st->getHouseBuildCost();
        if (!current.canAfford(cost)) {
            return false;
        }
        current.pay(cost);
        st->buildHouse();
        logger.log(turnManager.getCurrentTurn(), current.getUsername(), "BANGUN",
                   "Rumah di " + st->getName());
        return true;
    }

    if (st->getBuildingLevel() == 4 && st->canBuildHotel(this)) {
        int cost = st->getHotelBuildCost();
        if (!current.canAfford(cost)) {
            return false;
        }
        current.pay(cost);
        st->buildHotel();
        logger.log(turnManager.getCurrentTurn(), current.getUsername(), "BANGUN",
                   "Hotel di " + st->getName());
        return true;
    }

    return false;
}

namespace {
void liquidateAutomatically(Player& player, Game& game, int amount) {
    for (PropertyTile* prop : player.getOwnedProperties()) {
        if (player.canAfford(amount)) {
            return;
        }

        StreetTile* street = dynamic_cast<StreetTile*>(prop);
        if (street != nullptr && street->getBuildingLevel() > 0) {
            int funds = street->sellBuildingsToBank();
            player.receive(funds);
            game.getLogger().log(game.getTurnManager().getCurrentTurn(),
                                 player.getUsername(), "LIKUIDASI",
                                 "Jual bangunan " + prop->getName() + " M" + to_string(funds));
        }

        if (!player.canAfford(amount) && !prop->isMortgaged() && prop->canBeMortgaged(&game)) {
            int funds = prop->mortgage();
            player.receive(funds);
            game.getLogger().log(game.getTurnManager().getCurrentTurn(),
                                 player.getUsername(), "GADAI",
                                 prop->getName() + " M" + to_string(funds));
        }
    }
}
}

void Game::awardGoSalary(Player& player) {
    int salary = config.getSpecialConfig(GO_SALARY);
    if (salary <= 0) {
        salary = 200;
    }
    player.receive(salary);
    logger.log(turnManager.getCurrentTurn(), player.getUsername(), "GO", "M" + to_string(salary));
}

void Game::payBankOrBankrupt(Player& player, int amount, const std::string& reason) {
    if (amount <= 0 || player.isBankrupt()) {
        return;
    }

    if (!player.canAfford(amount)) {
        liquidateAutomatically(player, *this, amount);
    }

    if (!player.canAfford(amount)) {
        declareBankruptToBank(player);
        return;
    }

    player.pay(amount);
    logger.log(turnManager.getCurrentTurn(), player.getUsername(), "BAYAR_BANK",
               reason + " M" + to_string(amount));
}

void Game::payPlayerOrBankrupt(Player& payer, Player& receiver, int amount, const std::string& reason) {
    if (amount <= 0 || payer.isBankrupt() || receiver.isBankrupt()) {
        return;
    }

    if (!payer.canAfford(amount)) {
        liquidateAutomatically(payer, *this, amount);
    }

    if (!payer.canAfford(amount)) {
        declareBankruptToPlayer(payer, receiver);
        return;
    }

    payer.pay(amount);
    receiver.receive(amount);
    logger.log(turnManager.getCurrentTurn(), payer.getUsername(), "BAYAR_PEMAIN",
               reason + " ke " + receiver.getUsername() + " M" + to_string(amount));
}

void Game::declareBankruptToBank(Player& player) {
    if (player.isBankrupt()) {
        return;
    }
    bankruptcyManager.declareBankruptToBank(player, *this);
    player.setStatus(PlayerStatus::BANKRUPT);
    turnManager.removePlayer(player.getId());
    checkWinCondition();
}

void Game::declareBankruptToPlayer(Player& player, Player& creditor) {
    if (player.isBankrupt()) {
        return;
    }
    bankruptcyManager.declareBankruptToPlayer(player, creditor, *this);
    player.setStatus(PlayerStatus::BANKRUPT);
    turnManager.removePlayer(player.getId());
    checkWinCondition();
}

void Game::setLastDiceTotal(int total) {
    lastDiceTotal = total;
}

int Game::getLastDiceTotal() const {
    return lastDiceTotal;
}

bool Game::getHasRolledThisTurn() const {
    return hasRolledThisTurn;
}

bool Game::isExtraRollPending() const {
    return extraRollPending;
}

bool Game::prepareExtraRollForCurrentPlayer() {
    if (!extraRollPending || gameOver || players.empty()) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    if (current.getStatus() != PlayerStatus::ACTIVE || current.isBankrupt()) {
        extraRollPending = false;
        return false;
    }

    hasRolledThisTurn = false;
    extraRollPending = false;
    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "DADU",
               "Giliran ekstra siap");
    return true;
}

bool Game::payJailFineForCurrentPlayer() {
    if (players.empty() || gameOver || hasRolledThisTurn) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    if (current.getStatus() != PlayerStatus::JAILED) {
        return false;
    }

    int fine = config.getSpecialConfig(JAIL_FINE);
    payBankOrBankrupt(current, fine, "Denda penjara");
    if (current.isBankrupt()) {
        return false;
    }

    current.releaseFromJail();
    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "PENJARA",
               "Keluar dengan bayar denda M" + to_string(fine));
    return true;
}

bool Game::useJailFreeCardForCurrentPlayer() {
    if (players.empty() || gameOver || hasRolledThisTurn) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    if (current.getStatus() != PlayerStatus::JAILED) {
        return false;
    }

    const auto& hand = current.getHandCards();
    for (size_t i = 0; i < hand.size(); ++i) {
        if (dynamic_cast<JailFreeCard*>(hand[i].get()) != nullptr ||
            hand[i]->getName() == "JailFreeCard") {
            std::unique_ptr<AbilityCard> card = current.dropCard(static_cast<int>(i));
            card->use(&current, this);
            cardManager.discardAbilityCard(std::move(card));
            logger.log(turnManager.getCurrentTurn(), current.getUsername(), "PENJARA",
                       "Keluar dengan kartu Bebas dari Penjara");
            return true;
        }
    }

    return false;
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
    if (command == "LEMPAR_DADU") {
        rollDiceForCurrentPlayer();
    } else if (command == "AKHIRI_GILIRAN") {
        if (!prepareExtraRollForCurrentPlayer()) {
            endCurrentTurn();
        }
    } else if (command == "BELI") {
        buyCurrentProperty();
    }
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
