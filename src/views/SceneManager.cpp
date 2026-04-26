#include "../../include/views/SceneManager.hpp"
#include "../../include/views/InGameScene.hpp"
#include "../../include/views/MainMenuScene.hpp"

SceneManager::SceneManager() : SceneManager(nullptr) {}

SceneManager::SceneManager(IGameFacade* facade) : gameFacade(facade), currentScene(nullptr) {}

SceneManager::~SceneManager() = default;

void SceneManager::initialize(IGameFacade* facade) {
    gameFacade = facade;
    mainMenuScene = std::make_unique<MainMenuScene>(this, gameFacade);
    inGameScene = std::make_unique<InGameScene>(this, gameFacade);
    currentScene = mainMenuScene.get();
}

void SceneManager::setScene(SceneType type) {
    if (type == SceneType::MainMenu) {
        currentScene = mainMenuScene.get();
    } else {
        currentScene = inGameScene.get();
    }

    if (currentScene != nullptr) {
        currentScene->onEnter();
    }
}

void SceneManager::update() {
    if (currentScene != nullptr) {
        currentScene->update();
    }
}

void SceneManager::draw() {
    if (currentScene != nullptr) {
        currentScene->draw();
    }
}
