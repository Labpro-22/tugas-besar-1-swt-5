#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

class AbilityCard;
class Game;
class Player;
class ChanceCard;
class CommunityChestCard;
class Account;

class GameStateLoader
{
public:
    static bool load(Game& game, const std::string& filePath);

private:
    struct PlayerSaveData
    {
        int id = 0;
        std::string username;
        int money = 0;
        int position = 0;
        std::string status;
        int jailTurnsAttempted = 0;
        bool usedAbilityThisTurn = false;
        bool shieldActive = false;
        int discountPercent = 0;
        int discountDuration = 0;
        int consecutiveDoubleCount = 0;
        std::vector<std::string> handCards;
    };

    struct PropertySaveData
    {
        std::size_t boardIndex = 0;
        int ownerId = -1;
        std::string ownerUsername;
        std::string status;
        int festivalMultiplier = 1;
        int festivalDuration = 0;
    };

    struct LogSaveData
    {
        int turn = 0;
        std::string username;
        std::string actionType;
        std::string detail;
    };

    struct AbilityDeckSaveData
    {
        std::vector<std::string> drawPile;
        std::vector<std::string> discardedPile;
    };

    struct CardManagerSaveData
    {
        std::vector<std::string> chanceDeck;
        std::vector<std::string> communityDeck;
        AbilityDeckSaveData abilityDeck;
    };

    static std::string trim(const std::string& text);
    static bool startsWith(const std::string& text, const std::string& prefix);
    static bool parseKeyValue(const std::string& line, std::string& key, std::string& value);
    static std::string unescapeSaveValue(const std::string& value);
    static bool parseBool(const std::string& value);
    static int parseInt(const std::string& value, int defaultValue = 0);
    static std::size_t parseSize(const std::string& value, std::size_t defaultValue = 0);

    static bool findSection(const std::vector<std::string>& lines,
                            const std::string& sectionName,
                            std::size_t& begin,
                            std::size_t& end);

    static std::vector<std::string> readAllLines(const std::string& filePath);

    static std::vector<PlayerSaveData> parsePlayers(const std::vector<std::string>& lines);
    static std::vector<PropertySaveData> parseProperties(const std::vector<std::string>& lines);
    static CardManagerSaveData parseCardManager(const std::vector<std::string>& lines);
    static std::vector<LogSaveData> parseLogs(const std::vector<std::string>& lines);

    static std::vector<std::string> parseSimpleCardDeck(const std::vector<std::string>& lines,
                                                         const std::string& sectionName);
    static AbilityDeckSaveData parseAbilityDeck(const std::vector<std::string>& lines);

    static std::vector<std::string> splitEscaped(const std::string& text, char delimiter);
    static bool splitEscapedKeyValue(const std::string& text, std::string& key, std::string& value);
    static std::string unescapeSerializedCardValue(const std::string& value);
    static std::map<std::string, std::string> parseSerializedCardFields(const std::string& serializedCard);

    static std::unique_ptr<AbilityCard> createAbilityCard(const std::string& serializedCard);
    static std::vector<ChanceCard*> createChanceCards(const std::vector<std::string>& descriptions);
    static std::vector<CommunityChestCard*> createCommunityCards(const std::vector<std::string>& descriptions);

    static std::vector<std::unique_ptr<Account>>& accountPool();
    static std::vector<std::unique_ptr<ChanceCard>>& chanceCardPool();
    static std::vector<std::unique_ptr<CommunityChestCard>>& communityCardPool();

    static Player* findPlayerById(std::vector<Player>& players, int id);
    static Player* findPlayerByUsername(std::vector<Player>& players, const std::string& username);

    static void applyGameOver(Game& game, const std::vector<std::string>& lines);
    static void applyPlayers(Game& game, const std::vector<PlayerSaveData>& playersData);
    static void applyProperties(Game& game, const std::vector<PropertySaveData>& propertiesData);
    static void applyFestivalState(class PropertyTile* property, int multiplier, int duration);
    static void applyCardManager(Game& game, const CardManagerSaveData& cardManagerData);
    static void applyLogs(Game& game, const std::vector<LogSaveData>& logsData);
};
