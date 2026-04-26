#include "GameStateSaver.hpp"

#include "../../include/core/Game.hpp"
#include "../../include/utils/Board.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/models/DrawCardDeck.hpp"
#include "../../include/core/CardManager.hpp"
#include "../../include/utils/Logger.hpp"
#include "../../include/utils/LogEntry.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/utils/Tile.hpp"

#include <fstream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

std::string GameStateSaver::escapeValue(const std::string& value)
{
    std::string result;
    result.reserve(value.size());

    for (char c : value)
    {
        switch (c)
        {
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            case '=':  result += "\\="; break;
            default:   result += c; break;
        }
    }

    return result;
}

std::string GameStateSaver::boolToString(bool value)
{
    return value ? "true" : "false";
}

void GameStateSaver::writeRaw(std::ofstream& out, const std::string& text)
{
    out << text << '\n';
}

void GameStateSaver::writeString(std::ofstream& out, const std::string& key, const std::string& value)
{
    out << key << '=' << escapeValue(value) << '\n';
}

void GameStateSaver::writeInt(std::ofstream& out, const std::string& key, int value)
{
    out << key << '=' << value << '\n';
}

void GameStateSaver::writeSize(std::ofstream& out, const std::string& key, std::size_t value)
{
    out << key << '=' << value << '\n';
}

void GameStateSaver::writeBool(std::ofstream& out, const std::string& key, bool value)
{
    out << key << '=' << boolToString(value) << '\n';
}

std::string GameStateSaver::playerStatusToString(PlayerStatus status)
{
    switch (status)
    {
        case PlayerStatus::ACTIVE:   return "ACTIVE";
        case PlayerStatus::JAILED:   return "JAILED";
        case PlayerStatus::BANKRUPT: return "BANKRUPT";
        default:                     return "UNKNOWN";
    }
}

std::string GameStateSaver::propertyStatusToString(PropertyStatus status)
{
    switch (status)
    {
        case BANK:       return "BANK";
        case OWNED:      return "OWNED";
        case MORTGAGED:  return "MORTGAGED";
        default:         return "UNKNOWN";
    }
}

std::string GameStateSaver::serializeAbilityCard(const AbilityCard* card)
{
    if (card == nullptr)
    {
        return "NULL";
    }

    return card->serialize();
}

template <typename T>
std::string GameStateSaver::serializeDrawCard(const T* card)
{
    if (card == nullptr)
    {
        return "NULL";
    }

    return card->getDescription();
}

void GameStateSaver::savePlayers(std::ofstream& out, Game& game)
{
    std::vector<Player>& players = game.getPlayers();

    writeRaw(out, "[PLAYERS]");
    writeSize(out, "COUNT", players.size());

    for (std::size_t i = 0; i < players.size(); ++i)
    {
        Player& player = players[i];

        writeRaw(out, "BEGIN_PLAYER");
        writeSize(out, "INDEX", i);
        writeInt(out, "ID", player.getId());
        writeString(out, "USERNAME", player.getUsername());
        writeInt(out, "MONEY", player.getMoney());
        writeInt(out, "POSITION", player.getPosition());
        writeString(out, "STATUS", playerStatusToString(player.getStatus()));
        writeInt(out, "JAIL_TURNS_ATTEMPTED", player.getJailTurnsAttempted());
        writeBool(out, "USED_ABILITY_THIS_TURN", player.hasUsedAbilityThisTurn());
        writeBool(out, "SHIELD_ACTIVE", player.isShieldActive());
        writeInt(out, "DISCOUNT_PERCENT", player.getDiscountPercent());
        writeInt(out, "DISCOUNT_DURATION", player.getDiscountDuration());
        writeInt(out, "CONSECUTIVE_DOUBLE_COUNT", player.getConsecutiveDoubleCount());

        const std::vector<std::unique_ptr<AbilityCard>>& handCards = player.getHandCards();
        writeSize(out, "HAND_CARD_COUNT", handCards.size());

        for (std::size_t j = 0; j < handCards.size(); ++j)
        {
            writeString(out, "HAND_CARD_" + std::to_string(j), serializeAbilityCard(handCards[j].get()));
        }

        writeRaw(out, "END_PLAYER");
    }

    writeRaw(out, "[/PLAYERS]");
}

void GameStateSaver::saveProperties(std::ofstream& out, Game& game)
{
    const std::vector<Tile*>& tiles = game.getBoard().getTiles();
    std::vector<std::pair<std::size_t, PropertyTile*>> properties;

    for (std::size_t i = 0; i < tiles.size(); ++i)
    {
        PropertyTile* property = dynamic_cast<PropertyTile*>(tiles[i]);
        if (property != nullptr)
        {
            properties.push_back({i, property});
        }
    }

    writeRaw(out, "[PROPERTIES]");
    writeSize(out, "COUNT", properties.size());

    for (std::size_t i = 0; i < properties.size(); ++i)
    {
        const std::size_t boardIndex = properties[i].first;
        PropertyTile* property = properties[i].second;
        Player* owner = property->getOwner();

        writeRaw(out, "BEGIN_PROPERTY");
        writeSize(out, "INDEX", i);
        writeSize(out, "BOARD_INDEX", boardIndex);
        writeString(out, "OWNER_USERNAME", owner == nullptr ? "NONE" : owner->getUsername());
        writeInt(out, "OWNER_ID", owner == nullptr ? -1 : owner->getId());
        writeString(out, "STATUS", propertyStatusToString(property->getStatus()));
        writeInt(out, "MORTGAGE_VALUE", property->getMortgageValue());
        writeInt(out, "FESTIVAL_MULTIPLIER", property->getFestivalMultiplier());
        writeInt(out, "FESTIVAL_DURATION", property->getFestivalDuration());
        writeRaw(out, "END_PROPERTY");
    }

    writeRaw(out, "[/PROPERTIES]");
}

template <typename T>
void GameStateSaver::saveDrawCardDeck(std::ofstream& out, const std::string& sectionName, DrawCardDeck<T>& deck)
{
    const std::vector<T*>& pile = deck.getPile();

    writeRaw(out, "[" + sectionName + "]");
    writeSize(out, "COUNT", pile.size());

    for (std::size_t i = 0; i < pile.size(); ++i)
    {
        writeString(out, "CARD_" + std::to_string(i), serializeDrawCard(pile[i]));
    }

    writeRaw(out, "[/" + sectionName + "]");
}

void GameStateSaver::saveAbilityCardDeck(std::ofstream& out, AbilityCardDeck& deck)
{
    writeRaw(out, "[ABILITY_DECK]");

    const std::vector<std::unique_ptr<AbilityCard>>& drawPile = deck.getDrawPile();
    writeSize(out, "DRAW_PILE_COUNT", drawPile.size());
    for (std::size_t i = 0; i < drawPile.size(); ++i)
    {
        writeString(out, "DRAW_CARD_" + std::to_string(i), serializeAbilityCard(drawPile[i].get()));
    }

    const std::vector<std::unique_ptr<AbilityCard>>& discardedPile = deck.getDiscardedPile();
    writeSize(out, "DISCARDED_PILE_COUNT", discardedPile.size());
    for (std::size_t i = 0; i < discardedPile.size(); ++i)
    {
        writeString(out, "DISCARDED_CARD_" + std::to_string(i), serializeAbilityCard(discardedPile[i].get()));
    }

    writeRaw(out, "[/ABILITY_DECK]");
}

void GameStateSaver::saveCardManager(std::ofstream& out, Game& game)
{
    CardManager& cardManager = game.getCardManager();

    writeRaw(out, "[CARD_MANAGER]");
    saveDrawCardDeck(out, "CHANCE_DECK", cardManager.getChanceDeck());
    saveDrawCardDeck(out, "COMMUNITY_DECK", cardManager.getCommunityDeck());
    saveAbilityCardDeck(out, cardManager.getAbilityDeck());
    writeRaw(out, "[/CARD_MANAGER]");
}

void GameStateSaver::saveLogs(std::ofstream& out, Game& game)
{
    std::vector<LogEntry> entries = game.getLogger().getEntries();

    writeRaw(out, "[LOGS]");
    writeSize(out, "COUNT", entries.size());

    for (std::size_t i = 0; i < entries.size(); ++i)
    {
        const LogEntry& entry = entries[i];

        writeRaw(out, "BEGIN_LOG");
        writeSize(out, "INDEX", i);
        writeInt(out, "TURN", entry.getTurn());
        writeString(out, "USERNAME", entry.getUsername());
        writeString(out, "ACTION_TYPE", entry.getActionType());
        writeString(out, "DETAIL", entry.getDetail());
        writeRaw(out, "END_LOG");
    }

    writeRaw(out, "[/LOGS]");
}

bool GameStateSaver::save(Game& game, const std::string& filePath)
{
    std::ofstream out(filePath);

    if (!out.is_open())
    {
        return false;
    }

    writeRaw(out, "=== GAME STATE SAVE ===");
    writeBool(out, "GAME_OVER", game.isGameOver());

    savePlayers(out, game);
    saveProperties(out, game);
    saveCardManager(out, game);
    saveLogs(out, game);

    writeRaw(out, "=== END GAME STATE SAVE ===");

    return out.good();
}
