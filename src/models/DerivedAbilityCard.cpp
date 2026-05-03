#include "../../include/models/DerivedAbilityCard.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/core/Game.hpp"
#include "../../include/data-layer/Config.hpp"
#include "../../include/core/InvalidActionException.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/utils/Tile.hpp"

#include <iostream>
#include <limits>
#include <random>

std::string AbilityCard::escapeSerializedValue(const std::string& value)
{
    std::string result;
    result.reserve(value.size());

    for (char c : value)
    {
        switch (c)
        {
            case '\\': result += "\\\\"; break;
            case '|':  result += "\\|";  break;
            case '=':  result += "\\=";  break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:   result += c;      break;
        }
    }

    return result;
}

std::string AbilityCard::serializeCommonAbilityFields(
    const std::string& name,
    const std::string& description
)
{
    return "NAME=" + escapeSerializedValue(name) +
           "|DESCRIPTION=" + escapeSerializedValue(description);
}

// ========================================================
// MoveCard
// ========================================================

MoveCard::MoveCard()
    : AbilityCard("MoveCard", "Maju x petak."),
      steps(0) {}

MoveCard::MoveCard(int steps)
    : AbilityCard("MoveCard", "Maju " + std::to_string(steps) + " petak."),
      steps(steps) {}

void MoveCard::setup()
{
    std::mt19937 randomizer(std::random_device{}());
    std::uniform_int_distribution<int> picker(1, 12);

    steps = picker(randomizer);
    description = "Maju " + std::to_string(steps) + " petak.";
}

void MoveCard::use(Player* target, Game* game)
{
    if (target == nullptr || game == nullptr)
    {
        throw InvalidActionException("MOVE TARGET/GAME TIDAK VALID");
    }

    Board& board = game->getBoard();

    if (board.size() <= 0)
    {
        throw InvalidActionException("BOARD KOSONG");
    }

    bool passedGo = false;
    int dest = board.calculateNewPosition(target->getPosition(), steps, passedGo);

    target->moveTo(dest);

    if (passedGo)
    {
        int salary = game->getConfig().getSpecialConfig(GO_SALARY);
        target->receive(salary);
    }

    Tile* landedTile = board.getTileByIndex(dest);
    if (landedTile != nullptr)
    {
        landedTile->onLand(target, game);
    }
}

std::string MoveCard::serialize() const
{
    return serializeCommonAbilityFields(name, description) +
           "|STEPS=" + std::to_string(steps);
}

// ========================================================
// DiscountCard
// ========================================================

DiscountCard::DiscountCard()
    : AbilityCard("DiscountCard", "Dapatkan diskon selama 1 giliran."),
      percentage(0) {}

DiscountCard::DiscountCard(int percentage)
    : AbilityCard("DiscountCard", "Dapatkan diskon " + std::to_string(percentage) + "%% selama 1 giliran."),
      percentage(percentage) {}

void DiscountCard::setup()
{
    std::mt19937 randomizer(std::random_device{}());
    std::uniform_int_distribution<int> picker(1, 100);

    percentage = picker(randomizer);
    description = "Dapatkan diskon " + std::to_string(percentage) + "%% selama 1 giliran.";
}

void DiscountCard::use(Player* target, Game* game)
{
    (void)game;

    if (target == nullptr)
    {
        throw InvalidActionException("DISCOUNT TARGET TIDAK VALID");
    }

    target->applyDiscount(percentage);
}

std::string DiscountCard::serialize() const
{
    return serializeCommonAbilityFields(name, description) +
           "|PERCENTAGE=" + std::to_string(percentage);
}

// ========================================================
// ShieldCard
// ========================================================

ShieldCard::ShieldCard()
    : AbilityCard("ShieldCard", "Terlindung dari tagihan sewa maupun sanksi apapun yang merugikan selama 1 giliran.") {}

void ShieldCard::use(Player* target, Game* game)
{
    (void)game;

    if (target == nullptr)
    {
        throw InvalidActionException("SHIELD TARGET TIDAK VALID");
    }

    target->activateShield();
}

std::string ShieldCard::serialize() const
{
    return serializeCommonAbilityFields(name, description);
}

// Teleport Card

TeleportCard::TeleportCard()
    : AbilityCard("TeleportCard", "Bebas berpindah ke petak manapun di atas papan permainan.") {}

void TeleportCard::use(Player* target, Game* game)
{
    if (target == nullptr || game == nullptr)
    {
        throw InvalidActionException("TELEPORT TARGET/GAME TIDAK VALID");
    }

    int dest;

    if (!(std::cin >> dest))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw InvalidActionException("INPUT TELEPORT TIDAK VALID");
    }

    Board& board = game->getBoard();
    int boardSize = board.size();

    if (dest < 0 || dest >= boardSize)
    {
        throw InvalidActionException("TELEPORT KE LUAR BOARD");
    }

    target->moveTo(dest);

    Tile* landedTile = board.getTileByIndex(dest);
    if (landedTile != nullptr)
    {
        landedTile->onLand(target, game);
    }
}

std::string TeleportCard::serialize() const
{
    return serializeCommonAbilityFields(name, description);
}

// ========================================================
// LassoCard
// ========================================================

LassoCard::LassoCard()
    : AbilityCard("LassoCard", "Tarik satu pemain lain di depanmu ke posisimu.") {}

void LassoCard::use(Player* target, Game* game)
{
    if (target == nullptr || game == nullptr)
    {
        throw InvalidActionException("LASSO TARGET/GAME TIDAK VALID");
    }

    int id;

    if (!(std::cin >> id))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw InvalidActionException("INPUT LASSO TIDAK VALID");
    }

    Player* enemy = nullptr;

    for (Player& player : game->getPlayers())
    {
        if (player.getId() == id)
        {
            enemy = &player;
            break;
        }
    }

    if (enemy == nullptr || enemy == target || enemy->isBankrupt())
    {
        throw InvalidActionException("LASSO PLAYER TIDAK VALID");
    }

    enemy->moveTo(target->getPosition());

    Tile* landedTile = game->getBoard().getTileByIndex(target->getPosition());
    if (landedTile != nullptr)
    {
        landedTile->onLand(enemy, game);
    }
}

std::string LassoCard::serialize() const
{
    return serializeCommonAbilityFields(name, description);
}

// ========================================================
// DemolitionCard
// ========================================================

DemolitionCard::DemolitionCard()
    : AbilityCard("DemolitionCard", "Hancurkan satu properti milik pemain lain.") {}

void DemolitionCard::use(Player* target, Game* game)
{
    if (target == nullptr || game == nullptr)
    {
        throw InvalidActionException("DEMOLITION TARGET/GAME TIDAK VALID");
    }

    int dest;

    if (!(std::cin >> dest))
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw InvalidActionException("INPUT DEMOLISH TIDAK VALID");
    }

    Board& board = game->getBoard();
    int boardSize = board.size();

    if (dest < 0 || dest >= boardSize)
    {
        throw InvalidActionException("DEMOLISH KE LUAR BOARD");
    }

    StreetTile* property = dynamic_cast<StreetTile*>(board.getTileByIndex(dest));

    if (property == nullptr)
    {
        throw InvalidActionException("BUKAN STREET, TIDAK BISA DEMOLISH");
    }

    if (property->getOwner() == nullptr || property->getOwner() == target)
    {
        throw InvalidActionException("DEMOLISH HANYA UNTUK PROPERTI LAWAN");
    }

    property->demolish();
}

std::string DemolitionCard::serialize() const
{
    return serializeCommonAbilityFields(name, description);
}

// ========================================================
// JailFreeCard
// ========================================================

JailFreeCard::JailFreeCard()
    : AbilityCard("JailFreeCard", "Bebas dari Penjara.") {}

void JailFreeCard::use(Player* target, Game* game)
{
    (void)game;

    if (target == nullptr)
    {
        throw InvalidActionException("JAIL FREE TARGET TIDAK VALID");
    }

    if (target->getStatus() != PlayerStatus::JAILED)
    {
        throw InvalidActionException("PEMAIN TIDAK SEDANG DI PENJARA");
    }

    target->releaseFromJail();
}

std::string JailFreeCard::serialize() const
{
    return serializeCommonAbilityFields(name, description);
}
