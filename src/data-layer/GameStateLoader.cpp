#include "../../include/data-layer/GameStateLoader.hpp"

#include "../../include/core/Game.hpp"
#include "../../include/core/CardManager.hpp"
#include "../../include/core/Account.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/models/AbilityCardDeck.hpp"
#include "../../include/models/ChanceCard.hpp"
#include "../../include/models/CommunityChestCard.hpp"
#include "../../include/models/DerivedAbilityCard.hpp"
#include "../../include/models/DrawCardDeck.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/utils/Board.hpp"
#include "../../include/utils/Logger.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/utils/Tile.hpp"
#include "../../include/core/TurnManager.hpp"
#include "../../include/core/TradeManager.hpp"
#include "../../include/models/TradeToPlayer.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdexcept>
#include <utility>

std::string GameStateLoader::trim(const std::string& text)
{
    const std::string whitespace = " \t\n\r\f\v";
    const std::size_t start = text.find_first_not_of(whitespace);

    if (start == std::string::npos)
    {
        return "";
    }

    const std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(start, end - start + 1);
}

bool GameStateLoader::startsWith(const std::string& text, const std::string& prefix)
{
    return text.size() >= prefix.size() && text.compare(0, prefix.size(), prefix) == 0;
}

bool GameStateLoader::parseKeyValue(const std::string& line, std::string& key, std::string& value)
{
    const std::size_t separator = line.find('=');

    if (separator == std::string::npos)
    {
        return false;
    }

    key = trim(line.substr(0, separator));
    value = unescapeSaveValue(line.substr(separator + 1));
    return !key.empty();
}

std::string GameStateLoader::unescapeSaveValue(const std::string& value)
{
    std::string result;
    result.reserve(value.size());

    bool escaped = false;
    for (char c : value)
    {
        if (escaped)
        {
            switch (c)
            {
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case '\\': result += '\\'; break;
                case '=': result += '='; break;
                default: result += c; break;
            }
            escaped = false;
        }
        else if (c == '\\')
        {
            escaped = true;
        }
        else
        {
            result += c;
        }
    }

    if (escaped)
    {
        result += '\\';
    }

    return result;
}

bool GameStateLoader::parseBool(const std::string& value)
{
    std::string normalized = trim(value);
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    return normalized == "true" || normalized == "1" || normalized == "yes";
}

int GameStateLoader::parseInt(const std::string& value, int defaultValue)
{
    try
    {
        return std::stoi(trim(value));
    }
    catch (...)
    {
        return defaultValue;
    }
}

std::size_t GameStateLoader::parseSize(const std::string& value, std::size_t defaultValue)
{
    try
    {
        return static_cast<std::size_t>(std::stoull(trim(value)));
    }
    catch (...)
    {
        return defaultValue;
    }
}

bool GameStateLoader::findSection(const std::vector<std::string>& lines,
                                  const std::string& sectionName,
                                  std::size_t& begin,
                                  std::size_t& end)
{
    const std::string openTag = "[" + sectionName + "]";
    const std::string closeTag = "[/" + sectionName + "]";

    for (std::size_t i = 0; i < lines.size(); ++i)
    {
        if (trim(lines[i]) == openTag)
        {
            for (std::size_t j = i + 1; j < lines.size(); ++j)
            {
                if (trim(lines[j]) == closeTag)
                {
                    begin = i + 1;
                    end = j;
                    return true;
                }
            }
            return false;
        }
    }

    return false;
}

std::vector<std::string> GameStateLoader::readAllLines(const std::string& filePath)
{
    std::ifstream in(filePath);

    if (!in.is_open())
    {
        return {};
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(in, line))
    {
        lines.push_back(line);
    }

    return lines;
}

std::vector<GameStateLoader::PlayerSaveData> GameStateLoader::parsePlayers(const std::vector<std::string>& lines)
{
    std::size_t begin = 0;
    std::size_t end = 0;
    std::vector<PlayerSaveData> playersData;

    if (!findSection(lines, "PLAYERS", begin, end))
    {
        return playersData;
    }

    for (std::size_t i = begin; i < end; ++i)
    {
        if (trim(lines[i]) != "BEGIN_PLAYER")
        {
            continue;
        }

        PlayerSaveData data;
        std::size_t handCardCount = 0;

        ++i;
        while (i < end && trim(lines[i]) != "END_PLAYER")
        {
            std::string key;
            std::string value;

            if (parseKeyValue(lines[i], key, value))
            {
                if (key == "ID") data.id = parseInt(value);
                else if (key == "USERNAME") data.username = value;
                else if (key == "MONEY") data.money = parseInt(value);
                else if (key == "POSITION") data.position = parseInt(value);
                else if (key == "STATUS") data.status = value;
                else if (key == "JAIL_TURNS_ATTEMPTED") data.jailTurnsAttempted = parseInt(value);
                else if (key == "USED_ABILITY_THIS_TURN") data.usedAbilityThisTurn = parseBool(value);
                else if (key == "SHIELD_ACTIVE") data.shieldActive = parseBool(value);
                else if (key == "DISCOUNT_PERCENT") data.discountPercent = parseInt(value);
                else if (key == "DISCOUNT_DURATION") data.discountDuration = parseInt(value);
                else if (key == "CONSECUTIVE_DOUBLE_COUNT") data.consecutiveDoubleCount = parseInt(value);
                else if (key == "HAND_CARD_COUNT")
                {
                    handCardCount = parseSize(value);
                    data.handCards.resize(handCardCount);
                }
                else if (startsWith(key, "HAND_CARD_"))
                {
                    const std::size_t index = parseSize(key.substr(10), handCardCount);
                    if (index >= data.handCards.size())
                    {
                        data.handCards.resize(index + 1);
                    }
                    data.handCards[index] = value;
                }
            }

            ++i;
        }

        playersData.push_back(data);
    }

    return playersData;
}

std::vector<GameStateLoader::PropertySaveData> GameStateLoader::parseProperties(const std::vector<std::string>& lines)
{
    std::size_t begin = 0;
    std::size_t end = 0;
    std::vector<PropertySaveData> propertiesData;

    if (!findSection(lines, "PROPERTIES", begin, end))
    {
        return propertiesData;
    }

    for (std::size_t i = begin; i < end; ++i)
    {
        if (trim(lines[i]) != "BEGIN_PROPERTY")
        {
            continue;
        }

        PropertySaveData data;

        ++i;
        while (i < end && trim(lines[i]) != "END_PROPERTY")
        {
            std::string key;
            std::string value;

            if (parseKeyValue(lines[i], key, value))
            {
                if (key == "BOARD_INDEX") data.boardIndex = parseSize(value);
                else if (key == "OWNER_ID") data.ownerId = parseInt(value, -1);
                else if (key == "OWNER_USERNAME") data.ownerUsername = value;
                else if (key == "STATUS") data.status = value;
                else if (key == "FESTIVAL_MULTIPLIER") data.festivalMultiplier = parseInt(value, 1);
                else if (key == "FESTIVAL_DURATION") data.festivalDuration = parseInt(value, 0);
            }

            ++i;
        }

        propertiesData.push_back(data);
    }

    return propertiesData;
}

std::vector<std::string> GameStateLoader::parseSimpleCardDeck(const std::vector<std::string>& lines,
                                                              const std::string& sectionName)
{
    std::size_t begin = 0;
    std::size_t end = 0;
    std::vector<std::string> cards;

    if (!findSection(lines, sectionName, begin, end))
    {
        return cards;
    }

    std::size_t count = 0;

    for (std::size_t i = begin; i < end; ++i)
    {
        std::string key;
        std::string value;

        if (!parseKeyValue(lines[i], key, value))
        {
            continue;
        }

        if (key == "COUNT")
        {
            count = parseSize(value);
            cards.resize(count);
        }
        else if (startsWith(key, "CARD_"))
        {
            const std::size_t index = parseSize(key.substr(5), count);
            if (index >= cards.size())
            {
                cards.resize(index + 1);
            }
            cards[index] = value;
        }
    }

    return cards;
}

GameStateLoader::AbilityDeckSaveData GameStateLoader::parseAbilityDeck(const std::vector<std::string>& lines)
{
    std::size_t begin = 0;
    std::size_t end = 0;
    AbilityDeckSaveData deckData;

    if (!findSection(lines, "ABILITY_DECK", begin, end))
    {
        return deckData;
    }

    std::size_t drawCount = 0;
    std::size_t discardedCount = 0;

    for (std::size_t i = begin; i < end; ++i)
    {
        std::string key;
        std::string value;

        if (!parseKeyValue(lines[i], key, value))
        {
            continue;
        }

        if (key == "DRAW_PILE_COUNT")
        {
            drawCount = parseSize(value);
            deckData.drawPile.resize(drawCount);
        }
        else if (key == "DISCARDED_PILE_COUNT")
        {
            discardedCount = parseSize(value);
            deckData.discardedPile.resize(discardedCount);
        }
        else if (startsWith(key, "DRAW_CARD_"))
        {
            const std::size_t index = parseSize(key.substr(10), drawCount);
            if (index >= deckData.drawPile.size())
            {
                deckData.drawPile.resize(index + 1);
            }
            deckData.drawPile[index] = value;
        }
        else if (startsWith(key, "DISCARDED_CARD_"))
        {
            const std::size_t index = parseSize(key.substr(15), discardedCount);
            if (index >= deckData.discardedPile.size())
            {
                deckData.discardedPile.resize(index + 1);
            }
            deckData.discardedPile[index] = value;
        }
    }

    return deckData;
}

GameStateLoader::CardManagerSaveData GameStateLoader::parseCardManager(const std::vector<std::string>& lines)
{
    CardManagerSaveData cardManagerData;
    cardManagerData.chanceDeck = parseSimpleCardDeck(lines, "CHANCE_DECK");
    cardManagerData.communityDeck = parseSimpleCardDeck(lines, "COMMUNITY_DECK");
    cardManagerData.abilityDeck = parseAbilityDeck(lines);
    return cardManagerData;
}

std::vector<GameStateLoader::LogSaveData> GameStateLoader::parseLogs(const std::vector<std::string>& lines)
{
    std::size_t begin = 0;
    std::size_t end = 0;
    std::vector<LogSaveData> logsData;

    if (!findSection(lines, "LOGS", begin, end))
    {
        return logsData;
    }

    for (std::size_t i = begin; i < end; ++i)
    {
        if (trim(lines[i]) != "BEGIN_LOG")
        {
            continue;
        }

        LogSaveData data;

        ++i;
        while (i < end && trim(lines[i]) != "END_LOG")
        {
            std::string key;
            std::string value;

            if (parseKeyValue(lines[i], key, value))
            {
                if (key == "TURN") data.turn = parseInt(value);
                else if (key == "USERNAME") data.username = value;
                else if (key == "ACTION_TYPE") data.actionType = value;
                else if (key == "DETAIL") data.detail = value;
            }

            ++i;
        }

        logsData.push_back(data);
    }

    return logsData;
}

std::vector<std::string> GameStateLoader::splitEscaped(const std::string& text, char delimiter)
{
    std::vector<std::string> parts;
    std::string current;
    bool escaped = false;

    for (char c : text)
    {
        if (escaped)
        {
            current += '\\';
            current += c;
            escaped = false;
        }
        else if (c == '\\')
        {
            escaped = true;
        }
        else if (c == delimiter)
        {
            parts.push_back(current);
            current.clear();
        }
        else
        {
            current += c;
        }
    }

    if (escaped)
    {
        current += '\\';
    }

    parts.push_back(current);
    return parts;
}

bool GameStateLoader::splitEscapedKeyValue(const std::string& text, std::string& key, std::string& value)
{
    bool escaped = false;

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        const char c = text[i];

        if (escaped)
        {
            escaped = false;
            continue;
        }

        if (c == '\\')
        {
            escaped = true;
            continue;
        }

        if (c == '=')
        {
            key = text.substr(0, i);
            value = text.substr(i + 1);
            return !key.empty();
        }
    }

    return false;
}

std::string GameStateLoader::unescapeSerializedCardValue(const std::string& value)
{
    std::string result;
    result.reserve(value.size());

    bool escaped = false;
    for (char c : value)
    {
        if (escaped)
        {
            switch (c)
            {
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case '\\': result += '\\'; break;
                case '|': result += '|'; break;
                case '=': result += '='; break;
                default: result += c; break;
            }
            escaped = false;
        }
        else if (c == '\\')
        {
            escaped = true;
        }
        else
        {
            result += c;
        }
    }

    if (escaped)
    {
        result += '\\';
    }

    return result;
}

std::map<std::string, std::string> GameStateLoader::parseSerializedCardFields(const std::string& serializedCard)
{
    std::map<std::string, std::string> fields;
    const std::vector<std::string> parts = splitEscaped(serializedCard, '|');

    for (const std::string& part : parts)
    {
        std::string key;
        std::string value;

        if (splitEscapedKeyValue(part, key, value))
        {
            fields[key] = unescapeSerializedCardValue(value);
        }
    }

    return fields;
}

std::unique_ptr<AbilityCard> GameStateLoader::createAbilityCard(const std::string& serializedCard)
{
    if (serializedCard.empty() || serializedCard == "NULL")
    {
        return nullptr;
    }

    const std::map<std::string, std::string> fields = parseSerializedCardFields(serializedCard);
    const auto nameIt = fields.find("NAME");

    if (nameIt == fields.end())
    {
        return nullptr;
    }

    const std::string& name = nameIt->second;

    if (name == "ShieldCard") return std::make_unique<ShieldCard>();
    if (name == "TeleportCard") return std::make_unique<TeleportCard>();
    if (name == "LassoCard") return std::make_unique<LassoCard>();
    if (name == "DemolitionCard") return std::make_unique<DemolitionCard>();
    if (name == "JailFreeCard") return std::make_unique<JailFreeCard>();

    if (name == "MoveCard")
    {
        const auto stepsIt = fields.find("STEPS");
        return std::make_unique<MoveCard>(stepsIt == fields.end() ? 0 : parseInt(stepsIt->second));
    }

    if (name == "DiscountCard")
    {
        const auto percentageIt = fields.find("PERCENTAGE");
        return std::make_unique<DiscountCard>(percentageIt == fields.end() ? 0 : parseInt(percentageIt->second));
    }

    return nullptr;
}

std::vector<std::unique_ptr<Account>>& GameStateLoader::accountPool()
{
    static std::vector<std::unique_ptr<Account>> accounts;
    return accounts;
}

std::vector<std::unique_ptr<ChanceCard>>& GameStateLoader::chanceCardPool()
{
    static std::vector<std::unique_ptr<ChanceCard>> cards;
    return cards;
}

std::vector<std::unique_ptr<CommunityChestCard>>& GameStateLoader::communityCardPool()
{
    static std::vector<std::unique_ptr<CommunityChestCard>> cards;
    return cards;
}

std::vector<ChanceCard*> GameStateLoader::createChanceCards(const std::vector<std::string>& descriptions)
{
    std::vector<std::unique_ptr<ChanceCard>>& pool = chanceCardPool();
    pool.clear();

    std::vector<ChanceCard*> cards;
    cards.reserve(descriptions.size());

    for (const std::string& description : descriptions)
    {
        std::string body = description;
        const std::string prefix = "Kartu Kesempatan\n";
        if (startsWith(body, prefix))
        {
            body = body.substr(prefix.size());
        }

        pool.push_back(std::make_unique<ChanceCard>(body, [](Player*, Game*) {}));
        cards.push_back(pool.back().get());
    }

    return cards;
}

std::vector<CommunityChestCard*> GameStateLoader::createCommunityCards(const std::vector<std::string>& descriptions)
{
    std::vector<std::unique_ptr<CommunityChestCard>>& pool = communityCardPool();
    pool.clear();

    std::vector<CommunityChestCard*> cards;
    cards.reserve(descriptions.size());

    for (const std::string& description : descriptions)
    {
        std::string body = description;
        const std::string prefix = "Kartu Dana Umum\n";
        if (startsWith(body, prefix))
        {
            body = body.substr(prefix.size());
        }

        pool.push_back(std::make_unique<CommunityChestCard>(body, [](Player*, Game*) {}));
        cards.push_back(pool.back().get());
    }

    return cards;
}

Player* GameStateLoader::findPlayerById(std::vector<Player>& players, int id)
{
    for (Player& player : players)
    {
        if (player.getId() == id)
        {
            return &player;
        }
    }

    return nullptr;
}

Player* GameStateLoader::findPlayerByUsername(std::vector<Player>& players, const std::string& username)
{
    for (Player& player : players)
    {
        if (player.getUsername() == username)
        {
            return &player;
        }
    }

    return nullptr;
}

void GameStateLoader::applyGameOver(Game& game, const std::vector<std::string>& lines)
{
    for (const std::string& line : lines)
    {
        std::string key;
        std::string value;

        if (parseKeyValue(line, key, value) && key == "GAME_OVER")
        {
            game.setGameOver(parseBool(value));
            return;
        }
    }
}

void GameStateLoader::applyPlayers(Game& game, const std::vector<PlayerSaveData>& playersData)
{
    std::vector<std::unique_ptr<Account>>& accounts = accountPool();
    accounts.clear();

    std::vector<Player>& players = game.getPlayers();
    players.clear();

    for (const PlayerSaveData& data : playersData)
    {
        accounts.push_back(std::make_unique<Account>(data.username, ""));

        PlayerStatus status = PlayerStatus::ACTIVE;
        if (data.status == "JAILED") status = PlayerStatus::JAILED;
        else if (data.status == "BANKRUPT") status = PlayerStatus::BANKRUPT;

        std::vector<PropertyTile*> ownedProperties;
        std::vector<std::unique_ptr<AbilityCard>> handCards;

        for (const std::string& serializedCard : data.handCards)
        {
            std::unique_ptr<AbilityCard> card = createAbilityCard(serializedCard);
            if (card != nullptr)
            {
                handCards.push_back(std::move(card));
            }
        }

        players.emplace_back(
            data.id,
            accounts.back().get(),
            data.money,
            data.position,
            status,
            std::move(ownedProperties),
            std::move(handCards),
            data.jailTurnsAttempted,
            data.usedAbilityThisTurn,
            data.shieldActive,
            data.discountPercent,
            data.discountDuration,
            data.consecutiveDoubleCount
        );
    }
}

void GameStateLoader::applyFestivalState(PropertyTile* property, int multiplier, int duration)
{
    if (property == nullptr)
    {
        return;
    }

    while (property->getFestivalMultiplier() < multiplier)
    {
        property->applyFestival();
    }

    while (property->getFestivalDuration() > duration)
    {
        property->decrementFestivalDuration();
    }

    property->resetFestivalIfExpired();
}

void GameStateLoader::applyProperties(Game& game, const std::vector<PropertySaveData>& propertiesData)
{
    std::vector<Player>& players = game.getPlayers();

    for (Player& player : players)
    {
        player.getOwnedProperties().clear();
    }

    const std::vector<Tile*>& tiles = game.getBoard().getTiles();

    for (const PropertySaveData& data : propertiesData)
    {
        if (data.boardIndex >= tiles.size())
        {
            continue;
        }

        PropertyTile* property = dynamic_cast<PropertyTile*>(tiles[data.boardIndex]);
        if (property == nullptr)
        {
            continue;
        }

        Player* owner = nullptr;
        if (data.ownerId >= 0)
        {
            owner = findPlayerById(players, data.ownerId);
        }
        if (owner == nullptr && !data.ownerUsername.empty() && data.ownerUsername != "NONE")
        {
            owner = findPlayerByUsername(players, data.ownerUsername);
        }

        if (data.status == "BANK" || owner == nullptr)
        {
            property->setOwner(nullptr);
        }
        else
        {
            property->setOwner(owner);
            owner->addProperty(property);

            if (data.status == "MORTGAGED")
            {
                property->mortgage();
            }
        }

        applyFestivalState(property, data.festivalMultiplier, data.festivalDuration);
    }
}

void GameStateLoader::applyCardManager(Game& game, const CardManagerSaveData& cardManagerData)
{
    DrawCardDeck<ChanceCard> chanceDeck(createChanceCards(cardManagerData.chanceDeck));
    DrawCardDeck<CommunityChestCard> communityDeck(createCommunityCards(cardManagerData.communityDeck));

    std::vector<std::unique_ptr<AbilityCard>> drawPile;
    std::vector<std::unique_ptr<AbilityCard>> discardedPile;

    for (const std::string& serializedCard : cardManagerData.abilityDeck.drawPile)
    {
        std::unique_ptr<AbilityCard> card = createAbilityCard(serializedCard);
        if (card != nullptr)
        {
            drawPile.push_back(std::move(card));
        }
    }

    for (const std::string& serializedCard : cardManagerData.abilityDeck.discardedPile)
    {
        std::unique_ptr<AbilityCard> card = createAbilityCard(serializedCard);
        if (card != nullptr)
        {
            discardedPile.push_back(std::move(card));
        }
    }

    AbilityCardDeck abilityDeck(std::move(drawPile), std::move(discardedPile));

    CardManager& cardManager = game.getCardManager();
    cardManager.setChanceDeck(chanceDeck);
    cardManager.setCommunityDeck(communityDeck);
    cardManager.setAbilityDeck(std::move(abilityDeck));
}

void GameStateLoader::applyLogs(Game& game, const std::vector<LogSaveData>& logsData)
{
    Logger& logger = game.getLogger();
    logger.clear();

    for (const LogSaveData& data : logsData)
    {
        logger.log(data.turn, data.username, data.actionType, data.detail);
    }
}

bool GameStateLoader::load(Game& game, const std::string& filePath)
{
    const std::vector<std::string> lines = readAllLines(filePath);

    if (lines.empty())
    {
        return false;
    }

    if (trim(lines.front()) != "=== GAME STATE SAVE ===")
    {
        return false;
    }

    applyGameOver(game, lines);
    applyPlayers(game, parsePlayers(lines));
    applyProperties(game, parseProperties(lines));
    applyTurnManager(game, parseTurnManager(lines));
    applyCardManager(game, parseCardManager(lines));
    applyTradeManager(game, parseTradeManager(lines));
    applyLogs(game, parseLogs(lines));

    return true;
}

GameStateLoader::TurnManagerSaveData GameStateLoader::parseTurnManager(const std::vector<std::string>& lines)
{
    std::size_t begin = 0;
    std::size_t end = 0;
    TurnManagerSaveData data;

    if (!findSection(lines, "TURN_MANAGER", begin, end))
    {
        return data;
    }

    data.found = true;

    std::size_t turnOrderCount = 0;

    for (std::size_t i = begin; i < end; ++i)
    {
        std::string key;
        std::string value;

        if (!parseKeyValue(lines[i], key, value))
        {
            continue;
        }

        if (key == "CURRENT_PLAYER_INDEX")
        {
            data.currentPlayerIndex = parseInt(value);
        }
        else if (key == "CURRENT_TURN")
        {
            data.currentTurn = parseInt(value, 1);
        }
        else if (key == "MAX_TURN")
        {
            data.maxTurn = parseInt(value);
        }
        else if (key == "TURN_ORDER_COUNT")
        {
            turnOrderCount = parseSize(value);
            data.turnOrder.resize(turnOrderCount);
        }
        else if (startsWith(key, "TURN_ORDER_"))
        {
            const std::size_t index = parseSize(key.substr(11), turnOrderCount);

            if (index >= data.turnOrder.size())
            {
                data.turnOrder.resize(index + 1);
            }

            data.turnOrder[index] = parseInt(value);
        }
    }

    return data;
}

std::vector<GameStateLoader::TradeSaveData> GameStateLoader::parseTradeManager(const std::vector<std::string>& lines)
{
    std::size_t begin = 0;
    std::size_t end = 0;
    std::vector<TradeSaveData> tradesData;

    if (!findSection(lines, "TRADE_MANAGER", begin, end))
    {
        return tradesData;
    }

    for (std::size_t i = begin; i < end; ++i)
    {
        if (trim(lines[i]) != "BEGIN_TRADE")
        {
            continue;
        }

        TradeSaveData data;
        std::size_t offeredPropertyCount = 0;
        std::size_t requestedPropertyCount = 0;

        ++i;
        while (i < end && trim(lines[i]) != "END_TRADE")
        {
            std::string key;
            std::string value;

            if (parseKeyValue(lines[i], key, value))
            {
                if (key == "PROPOSER_ID")
                {
                    data.proposerId = parseInt(value, -1);
                }
                else if (key == "PROPOSER_USERNAME")
                {
                    data.proposerUsername = value;
                }
                else if (key == "TARGET_ID")
                {
                    data.targetId = parseInt(value, -1);
                }
                else if (key == "TARGET_USERNAME")
                {
                    data.targetUsername = value;
                }
                else if (key == "OFFERED_MONEY")
                {
                    data.offeredMoney = parseInt(value);
                }
                else if (key == "REQUESTED_MONEY")
                {
                    data.requestedMoney = parseInt(value);
                }
                else if (key == "OFFERED_PROPERTY_COUNT")
                {
                    offeredPropertyCount = parseSize(value);
                    data.offeredPropertyCodes.resize(offeredPropertyCount);
                }
                else if (key == "REQUESTED_PROPERTY_COUNT")
                {
                    requestedPropertyCount = parseSize(value);
                    data.requestedPropertyCodes.resize(requestedPropertyCount);
                }
                else if (startsWith(key, "OFFERED_PROPERTY_"))
                {
                    const std::size_t index = parseSize(key.substr(17), offeredPropertyCount);

                    if (index >= data.offeredPropertyCodes.size())
                    {
                        data.offeredPropertyCodes.resize(index + 1);
                    }

                    data.offeredPropertyCodes[index] = value;
                }
                else if (startsWith(key, "REQUESTED_PROPERTY_"))
                {
                    const std::size_t index = parseSize(key.substr(19), requestedPropertyCount);

                    if (index >= data.requestedPropertyCodes.size())
                    {
                        data.requestedPropertyCodes.resize(index + 1);
                    }

                    data.requestedPropertyCodes[index] = value;
                }
            }

            ++i;
        }

        tradesData.push_back(data);
    }

    return tradesData;
}

PropertyTile* GameStateLoader::findPropertyByCode(Game& game, const std::string& code)
{
    if (code.empty() || code == "NONE")
    {
        return nullptr;
    }

    try
    {
        Tile* tile = game.getBoard().getTileByCode(code);
        return dynamic_cast<PropertyTile*>(tile);
    }
    catch (...)
    {
        return nullptr;
    }
}

void GameStateLoader::applyTurnManager(Game& game, const TurnManagerSaveData& turnManagerData)
{
    if (!turnManagerData.found)
    {
        return;
    }

    std::vector<int> turnOrder = turnManagerData.turnOrder;

    if (turnOrder.empty())
    {
        std::vector<Player>& players = game.getPlayers();

        for (std::size_t i = 0; i < players.size(); ++i)
        {
            turnOrder.push_back(static_cast<int>(i));
        }
    }

    game.getTurnManager() = TurnManager(turnOrder, turnManagerData.maxTurn);
    game.getTurnManager().setCurrentPlayerIndex(turnManagerData.currentPlayerIndex);
    game.getTurnManager().setCurrentTurn(turnManagerData.currentTurn);
}

void GameStateLoader::applyTradeManager(Game& game, const std::vector<TradeSaveData>& tradesData)
{
    TradeManager& tradeManager = game.getTradeManager();
    tradeManager.clearActiveTrades();

    std::vector<Player>& players = game.getPlayers();

    for (const TradeSaveData& data : tradesData)
    {
        Player* proposer = nullptr;
        Player* target = nullptr;

        if (data.proposerId >= 0)
        {
            proposer = findPlayerById(players, data.proposerId);
        }

        if (proposer == nullptr && !data.proposerUsername.empty() && data.proposerUsername != "NONE")
        {
            proposer = findPlayerByUsername(players, data.proposerUsername);
        }

        if (data.targetId >= 0)
        {
            target = findPlayerById(players, data.targetId);
        }

        if (target == nullptr && !data.targetUsername.empty() && data.targetUsername != "NONE")
        {
            target = findPlayerByUsername(players, data.targetUsername);
        }

        if (proposer == nullptr || target == nullptr)
        {
            continue;
        }

        std::vector<PropertyTile*> offeredProperties;
        std::vector<PropertyTile*> requestedProperties;

        bool valid = true;

        for (const std::string& code : data.offeredPropertyCodes)
        {
            PropertyTile* property = findPropertyByCode(game, code);

            if (property == nullptr)
            {
                valid = false;
                break;
            }

            offeredProperties.push_back(property);
        }

        if (!valid)
        {
            continue;
        }

        for (const std::string& code : data.requestedPropertyCodes)
        {
            PropertyTile* property = findPropertyByCode(game, code);

            if (property == nullptr)
            {
                valid = false;
                break;
            }

            requestedProperties.push_back(property);
        }

        if (!valid)
        {
            continue;
        }

        tradeManager.restoreTrade(
            proposer,
            target,
            offeredProperties,
            data.offeredMoney,
            requestedProperties,
            data.requestedMoney
        );
    }
}
