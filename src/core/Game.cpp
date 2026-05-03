#include "../../include/core/Game.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/models/DerivedAbilityCard.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/core/HandOverflowException.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>
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

std::string moneyText(int amount) {
    return "M" + std::to_string(amount);
}

std::string upperCopy(std::string value) {
    for (char& c : value) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return value;
}

std::string propertySummaryLine(PropertyTile* property, Game& game) {
    if (property == nullptr) {
        return "";
    }

    std::ostringstream oss;
    oss << property->toString();
    if (property->getFestivalMultiplier() > 1) {
        oss << " | Festival aktif: x" << property->getFestivalMultiplier()
            << ", sisa " << property->getFestivalDuration() << " giliran"
            << ", sewa terbaru=" << moneyText(property->calculateRent(nullptr, &game));
    }
    return oss.str();
}

std::vector<StreetTile*> collectMutableStreetGroup(Game& game, const std::string& colorGroup) {
    std::vector<StreetTile*> group;

    for (Tile* tile : game.getBoard().getTiles()) {
        StreetTile* street = dynamic_cast<StreetTile*>(tile);
        if (street != nullptr && street->getColorGroup() == colorGroup) {
            group.push_back(street);
        }
    }

    return group;
}

int sellColorGroupBuildingsToBank(Game& game, Player& player, StreetTile& selected) {
    int total = 0;
    const std::vector<StreetTile*> group = collectMutableStreetGroup(game, selected.getColorGroup());

    for (StreetTile* street : group) {
        if (street == nullptr || street->getOwner() != &player || street->getBuildingLevel() == 0) {
            continue;
        }

        const int funds = street->sellBuildingsToBank();
        total += funds;
        player.receive(funds);
        game.getLogger().log(game.getTurnManager().getCurrentTurn(),
                             player.getUsername(), "JUAL_BANGUNAN",
                             street->getName() + " M" + to_string(funds));
    }

    return total;
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
      hasRolledThisTurn(false) {}

void Game::giveCurrentPlayerTurnStartAbility() {
    if (players.empty() || gameOver) return;

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) return;

    Player& current = players[static_cast<size_t>(playerIdx)];
    if (current.isBankrupt()) return;

    current.resetTurnFlags();

    std::unique_ptr<AbilityCard> card = cardManager.drawAbilityCard();
    std::string cardName = card ? card->getName() : "Ability Card";
    current.addCard(std::move(card));

    logger.log(
        turnManager.getCurrentTurn(),
        current.getUsername(),
        "KARTU_DAPAT",
        "Mendapat " + cardName + " di awal giliran"
    );

    if (current.getHandCards().size() > 3) {
        throw HandOverflowException(
            current.getUsername() + " memiliki lebih dari 3 kartu. Buang satu kartu."
        );
    }
}

// ──────────────────────────────────────────────
// GAME LOOP
// ──────────────────────────────────────────────

void Game::run() {
    while (!gameOver) {
        startTurn();
        break;
    }
}

void Game::startTurn() {
    if (players.empty() || gameOver) return;
    hasRolledThisTurn = false;

    giveCurrentPlayerTurnStartAbility();
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

    if (festivalSelectionPending && pendingFestivalPlayerId == current.getId()) {
        cout << "Selesaikan pilihan Festival terlebih dahulu.\n";
        return;
    }

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

    if (!gameOver) {
        giveCurrentPlayerTurnStartAbility();
    }
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

    if (festivalSelectionPending && pendingFestivalPlayerId == current.getId()) {
        cout << "Selesaikan pilihan Festival terlebih dahulu.\n";
        return {0, 0};
    }

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
            if (current.getJailTurnsAttempted() > 3) {
                int fine = config.getSpecialConfig(JAIL_FINE);
                payBankOrBankrupt(current, fine, "Denda penjara");
                if (!current.isBankrupt()) {
                    current.releaseFromJail();
                }
            }
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

    if (current.getDiscountDuration() > 0) {
        price = price * (100 - current.getDiscountPercent()) / 100;
    }

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
    if (pt == nullptr || pt->getOwner() != &current || pt->getStatus() != OWNED) {
        return false;
    }

    StreetTile* street = dynamic_cast<StreetTile*>(pt);
    if (street != nullptr) {
        const int buildingFunds = sellColorGroupBuildingsToBank(*this, current, *street);
        if (buildingFunds > 0) {
            logger.log(turnManager.getCurrentTurn(), current.getUsername(), "GADAI",
                       "Bangunan color group " + street->getColorGroup() +
                       " dijual M" + to_string(buildingFunds));
        }
    }

    if (!pt->canBeMortgaged(this)) {
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

    if (current.getDiscountDuration() > 0) {
        price = price * (100 - current.getDiscountPercent()) / 100;
    }
    
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
        if (current.getDiscountDuration() > 0) {
            cost = cost * (100 - current.getDiscountPercent()) / 100;
        }
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
        if (current.getDiscountDuration() > 0) {
            cost = cost * (100 - current.getDiscountPercent()) / 100;
        }
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

bool Game::useCurrentPlayerAbilityCard(int cardIndex) {
    if (players.empty() || gameOver || hasRolledThisTurn) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    if (current.isBankrupt() || current.hasUsedAbilityThisTurn()) {
        return false;
    }

    const auto& hand = current.getHandCards();
    if (cardIndex < 0 || cardIndex >= static_cast<int>(hand.size())) {
        return false;
    }

    if (!hand[static_cast<size_t>(cardIndex)]->canUse(&current, this)) {
        return false;
    }

    std::string cardName = hand[static_cast<size_t>(cardIndex)]->getName();
    if (cardName == "TeleportCard" || cardName == "LassoCard" || cardName == "DemolitionCard") {
        return false;
    }

    current.useAbilityCard(cardIndex, this);
    std::unique_ptr<AbilityCard> used = current.dropCard(cardIndex);
    cardManager.discardAbilityCard(std::move(used));

    logger.log(
        turnManager.getCurrentTurn(),
        current.getUsername(),
        "KARTU_PAKAI",
        "Memakai " + cardName
    );

    return true;
}

bool Game::useCurrentPlayerTeleportCard(int cardIndex, int tileIndex) {
    if (players.empty() || gameOver || hasRolledThisTurn || board.size() == 0) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    const auto& hand = current.getHandCards();
    if (current.isBankrupt() || current.hasUsedAbilityThisTurn() ||
        cardIndex < 0 || cardIndex >= static_cast<int>(hand.size()) ||
        tileIndex < 0 || tileIndex >= board.size()) {
        return false;
    }

    TeleportCard* card = dynamic_cast<TeleportCard*>(hand[static_cast<size_t>(cardIndex)].get());
    if (card == nullptr || !card->canUse(&current, this)) {
        return false;
    }

    std::string cardName = card->getName();
    card->setDestination(tileIndex);
    current.useAbilityCard(cardIndex, this);
    std::unique_ptr<AbilityCard> used = current.dropCard(cardIndex);
    cardManager.discardAbilityCard(std::move(used));

    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "KARTU_PAKAI",
               "Memakai " + cardName + " ke petak " + std::to_string(tileIndex));

    return true;
}

bool Game::useCurrentPlayerDemolitionCard(int cardIndex, int tileIndex) {
    if (players.empty() || gameOver || hasRolledThisTurn || board.size() == 0) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    const auto& hand = current.getHandCards();
    if (current.isBankrupt() || current.hasUsedAbilityThisTurn() ||
        cardIndex < 0 || cardIndex >= static_cast<int>(hand.size()) ||
        tileIndex < 0 || tileIndex >= board.size()) {
        return false;
    }

    DemolitionCard* card = dynamic_cast<DemolitionCard*>(hand[static_cast<size_t>(cardIndex)].get());
    if (card == nullptr || !card->canUse(&current, this)) {
        return false;
    }

    std::string cardName = card->getName();
    card->setTargetTileIndex(tileIndex);
    current.useAbilityCard(cardIndex, this);
    std::unique_ptr<AbilityCard> used = current.dropCard(cardIndex);
    cardManager.discardAbilityCard(std::move(used));

    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "KARTU_PAKAI",
               "Memakai " + cardName + " di petak " + std::to_string(tileIndex));

    return true;
}

bool Game::useCurrentPlayerLassoCard(int cardIndex, int targetPlayerId) {
    if (players.empty() || gameOver || hasRolledThisTurn) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    const auto& hand = current.getHandCards();
    if (current.isBankrupt() || current.hasUsedAbilityThisTurn() ||
        cardIndex < 0 || cardIndex >= static_cast<int>(hand.size())) {
        return false;
    }

    LassoCard* card = dynamic_cast<LassoCard*>(hand[static_cast<size_t>(cardIndex)].get());
    if (card == nullptr || !card->canUse(&current, this)) {
        return false;
    }

    std::string cardName = card->getName();
    card->setTargetPlayerId(targetPlayerId);
    current.useAbilityCard(cardIndex, this);
    std::unique_ptr<AbilityCard> used = current.dropCard(cardIndex);
    cardManager.discardAbilityCard(std::move(used));

    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "KARTU_PAKAI",
               "Memakai " + cardName + " ke pemain " + std::to_string(targetPlayerId));

    return true;
}

bool Game::discardCurrentPlayerAbilityCard(int cardIndex) {
    if (players.empty() || gameOver) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    Player& current = players[static_cast<size_t>(playerIdx)];
    const auto& hand = current.getHandCards();
    if (cardIndex < 0 || cardIndex >= static_cast<int>(hand.size())) {
        return false;
    }

    std::unique_ptr<AbilityCard> dropped = current.dropCard(cardIndex);
    std::string cardName = dropped ? dropped->getName() : "Ability Card";
    cardManager.discardAbilityCard(std::move(dropped));

    logger.log(
        turnManager.getCurrentTurn(),
        current.getUsername(),
        "KARTU_BUANG",
        "Membuang " + cardName
    );

    return true;
}

namespace {
void liquidateAutomatically(Player& player, Game& game, int amount) {
    for (PropertyTile* prop : player.getOwnedProperties()) {
        if (player.canAfford(amount)) {
            return;
        }

        StreetTile* street = dynamic_cast<StreetTile*>(prop);
        if (street != nullptr && street->getBuildingLevel() > 0) {
            int funds = sellColorGroupBuildingsToBank(game, player, *street);
            game.getLogger().log(game.getTurnManager().getCurrentTurn(),
                                 player.getUsername(), "LIKUIDASI",
                                 "Jual bangunan color group " + street->getColorGroup() +
                                 " M" + to_string(funds));
        }

        if (!player.canAfford(amount) && prop->getStatus() == OWNED) {
            StreetTile* street = dynamic_cast<StreetTile*>(prop);
            if (street != nullptr && !prop->canBeMortgaged(&game)) {
                continue;
            }

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

    if (player.getDiscountDuration() > 0) {
        amount = amount * (100 - player.getDiscountPercent()) / 100;
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

    if (payer.getDiscountDuration() > 0) {
        amount = amount * (100 - payer.getDiscountPercent()) / 100;
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

void Game::requestFestivalSelection(Player& player) {
    std::vector<PropertyTile*>& properties = player.getOwnedProperties();

    cout << "Kamu mendarat di petak Festival!\n";
    if (properties.empty()) {
        cout << "Festival tidak aktif karena " << player.getUsername()
             << " belum memiliki properti.\n";
        logger.log(turnManager.getCurrentTurn(), player.getUsername(), "FESTIVAL", "Tidak ada properti");
        festivalSelectionPending = false;
        pendingFestivalPlayerId = -1;
        return;
    }

    festivalSelectionPending = true;
    pendingFestivalPlayerId = player.getId();

    for (const string& line : buildFestivalSelectionLines(player)) {
        cout << line << "\n";
    }
    cout << "Masukkan kode properti: ";
}

bool Game::hasPendingFestivalSelection() const {
    return festivalSelectionPending;
}

bool Game::isFestivalSelectionPendingForCurrentPlayer() const {
    if (!festivalSelectionPending || pendingFestivalPlayerId < 0 || players.empty()) {
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    if (playerIdx < 0 || playerIdx >= static_cast<int>(players.size())) {
        return false;
    }

    return players[static_cast<size_t>(playerIdx)].getId() == pendingFestivalPlayerId;
}

std::vector<std::string> Game::buildFestivalSelectionLines(Player& player) const {
    std::vector<std::string> lines;
    lines.push_back("Daftar properti milikmu:");

    for (PropertyTile* property : player.getOwnedProperties()) {
        if (property == nullptr) {
            continue;
        }
        lines.push_back("- " + property->getCode() + " (" + property->getName() + ")");
    }

    if (lines.size() == 1) {
        lines.push_back("(tidak ada properti)");
    }

    return lines;
}

bool Game::applyFestivalToCurrentPlayerProperty(const std::string& code, std::vector<std::string>* messages) {
    auto addMessage = [messages](const std::string& message) {
        if (messages != nullptr) {
            messages->push_back(message);
        }
    };

    if (!isFestivalSelectionPendingForCurrentPlayer()) {
        addMessage("Tidak ada pilihan Festival yang sedang menunggu.");
        return false;
    }

    int playerIdx = turnManager.getCurrentPlayerIndex();
    Player& current = players[static_cast<size_t>(playerIdx)];
    const std::string wantedCode = upperCopy(code);

    PropertyTile* selected = nullptr;
    try {
        Tile* tile = board.getTileByCode(wantedCode);
        selected = dynamic_cast<PropertyTile*>(tile);
    } catch (...) {
        selected = nullptr;
    }

    if (selected == nullptr) {
        addMessage("-> Kode properti tidak valid!");
        return false;
    }

    if (selected->getOwner() != &current) {
        addMessage("-> Properti bukan milikmu!");
        return false;
    }

    const int previousRent = selected->calculateRent(nullptr, this);
    const int previousMultiplier = selected->getFestivalMultiplier();

    selected->applyFestival();

    const int currentRent = selected->calculateRent(nullptr, this);
    const int currentMultiplier = selected->getFestivalMultiplier();

    if (previousMultiplier <= 1) {
        addMessage("Efek festival aktif!");
        addMessage("Sewa awal: " + moneyText(previousRent));
        addMessage("Sewa sekarang: " + moneyText(currentRent));
        addMessage("Durasi: " + std::to_string(selected->getFestivalDuration()) + " giliran");
    } else if (previousMultiplier < 8) {
        addMessage("Efek diperkuat!");
        addMessage("Sewa sebelumnya: " + moneyText(previousRent));
        addMessage("Sewa sekarang: " + moneyText(currentRent));
        addMessage("Durasi di-reset menjadi: " + std::to_string(selected->getFestivalDuration()) + " giliran");
    } else {
        addMessage("Efek sudah maksimum (harga sewa sudah digandakan tiga kali)");
        addMessage("Sewa sekarang: " + moneyText(currentRent));
        addMessage("Durasi di-reset menjadi: " + std::to_string(selected->getFestivalDuration()) + " giliran");
    }

    logger.log(turnManager.getCurrentTurn(), current.getUsername(), "FESTIVAL",
               selected->getName() + " x" + std::to_string(currentMultiplier) +
               " durasi " + std::to_string(selected->getFestivalDuration()));

    festivalSelectionPending = false;
    pendingFestivalPlayerId = -1;
    return true;
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
        if (!isFestivalSelectionPendingForCurrentPlayer() && !prepareExtraRollForCurrentPlayer()) {
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
