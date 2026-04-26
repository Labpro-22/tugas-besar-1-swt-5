#pragma once
#include <cstddef>
#include <fstream>
#include <string>

class AbilityCard;
class AbilityCardDeck;
class Game;

template <typename T>
class DrawCardDeck;

enum class PlayerStatus;
enum class PropertyStatus;

class GameStateSaver
{
public:
    static bool save(Game& game, const std::string& filePath);

private:
    static std::string escapeValue(const std::string& value);
    static std::string boolToString(bool value);

    static void writeRaw(std::ofstream& out, const std::string& text);
    static void writeString(std::ofstream& out, const std::string& key, const std::string& value);
    static void writeInt(std::ofstream& out, const std::string& key, int value);
    static void writeSize(std::ofstream& out, const std::string& key, std::size_t value);
    static void writeBool(std::ofstream& out, const std::string& key, bool value);

    static std::string playerStatusToString(PlayerStatus status);
    static std::string propertyStatusToString(PropertyStatus status);

    static std::string serializeAbilityCard(const AbilityCard* card);

    template <typename T>
    static std::string serializeDrawCard(const T* card);

    static void savePlayers(std::ofstream& out, Game& game);
    static void saveProperties(std::ofstream& out, Game& game);

    template <typename T>
    static void saveDrawCardDeck(std::ofstream& out, const std::string& sectionName, DrawCardDeck<T>& deck);

    static void saveAbilityCardDeck(std::ofstream& out, AbilityCardDeck& deck);
    static void saveCardManager(std::ofstream& out, Game& game);
    static void saveLogs(std::ofstream& out, Game& game);
};
