#include "../../include/core/GameManager.hpp"

#include "../../include/core/Game.hpp"
#include "../../include/data-layer/Config.hpp"
#include "../../include/data-layer/ConfigComposer.hpp"
#include "../../include/data-layer/FileIOException.hpp"
#include "../../include/data-layer/GameStateLoader.hpp"
#include "../../include/data-layer/GameStateSaver.hpp"
#include "../../include/utils/BoardBuilder.hpp"

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

static const std::string CONFIG_DIR = "config/basic";

static std::string joinConfigPath(const std::string& directory, const std::string& fileName)
{
    if (directory.empty()) {
        return fileName;
    }

    const char last = directory[directory.size() - 1];
    if (last == '/' || last == '\\') {
        return directory + fileName;
    }

    return directory + "/" + fileName;
}

static Config loadConfigFromDirectory(const std::string& configDirectory)
{
    ConfigComposer composer(
        joinConfigPath(configDirectory, "property.txt"),
        joinConfigPath(configDirectory, "railroad.txt"),
        joinConfigPath(configDirectory, "utility.txt"),
        joinConfigPath(configDirectory, "tax.txt"),
        joinConfigPath(configDirectory, "aksi.txt"),
        joinConfigPath(configDirectory, "special.txt"),
        joinConfigPath(configDirectory, "misc.txt")
    );

    return composer.getConfig();
}

static std::map<int, int> makeRailroadRentMap(Config& config)
{
    std::map<int, int> railroadRent;

    railroadRent[1] = config.getRailroadRent(1);
    railroadRent[2] = config.getRailroadRent(2);
    railroadRent[3] = config.getRailroadRent(3);
    railroadRent[4] = config.getRailroadRent(4);

    return railroadRent;
}

static std::map<int, int> makeUtilityMultiplierMap(Config& config)
{
    std::map<int, int> utilityMultiplier;

    utilityMultiplier[1] = config.getUtilityMultiplier(1);
    utilityMultiplier[2] = config.getUtilityMultiplier(2);

    return utilityMultiplier;
}

static void initializeGameFromConfig(Game& game, const std::string& configDirectory)
{
    Config config = loadConfigFromDirectory(configDirectory);
    game.setConfig(config);

    Config& gameConfig = game.getConfig();

    BoardBuilder::build(
        game.getBoard(),
        gameConfig.getPropertyConfigAll(),
        gameConfig.getActionTileConfigAll(),
        makeRailroadRentMap(gameConfig),
        makeUtilityMultiplierMap(gameConfig)
    );
}

GameManager::GameManager() : currentGame(nullptr) {}

GameManager::~GameManager()
{
    quitCurrentGame();
}

void GameManager::startNewGame()
{
    startNewGame(CONFIG_DIR);
}

void GameManager::startNewGame(const string& configDirectory)
{
    quitCurrentGame();

    currentGame = new Game();
    initializeGameFromConfig(*currentGame, configDirectory);
}

void GameManager::loadGame(const string& fileName)
{
    {
        std::ifstream input(fileName);

        if (!input.is_open())
        {
            throw FileIOException(
                "Gagal memuat game: file tidak ditemukan atau tidak dapat dibuka: " + fileName
            );
        }
    }

    std::unique_ptr<Game> loadedGame = std::make_unique<Game>();

    initializeGameFromConfig(*loadedGame, CONFIG_DIR);

    const bool success = GameStateLoader::load(*loadedGame, fileName);

    if (!success)
    {
        throw FileIOException(
            "Gagal memuat game: format save file tidak valid atau data tidak lengkap: " + fileName
        );
    }

    loadedGame->getLogger().log(
        0,
        "System",
        "LOAD",
        "Game berhasil dimuat dari " + fileName
    );

    quitCurrentGame();
    currentGame = loadedGame.release();
}

void GameManager::saveGame(const string& fileName) const
{
    if (currentGame == nullptr)
    {
        throw FileIOException("Gagal menyimpan game: tidak ada game aktif.");
    }

    const bool success = GameStateSaver::save(*currentGame, fileName);

    if (!success)
    {
        throw FileIOException(
            "Gagal menyimpan game: file tidak dapat ditulis: " + fileName
        );
    }

    currentGame->getLogger().log(
        currentGame->getTurnManager().getCurrentTurn(),
        "System",
        "SAVE",
        "Game disimpan ke " + fileName
    );
}

void GameManager::quitCurrentGame()
{
    delete currentGame;
    currentGame = nullptr;
}

Game* GameManager::getCurrentGame() const
{
    return currentGame;
}