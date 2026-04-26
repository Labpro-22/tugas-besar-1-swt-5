#ifndef SCENE_MANAGER_HPP
#define SCENE_MANAGER_HPP

#include <memory>

class IGameFacade;
class Scene;
class MainMenuScene;
class InGameScene;

enum class SceneType {
    MainMenu,
    InGame
};

class SceneManager {
public:
    SceneManager();
    explicit SceneManager(IGameFacade* facade);
    ~SceneManager();

    void initialize(IGameFacade* facade);
    void setScene(SceneType type);
    void update();
    void draw();

private:
    IGameFacade* gameFacade;
    std::unique_ptr<MainMenuScene> mainMenuScene;
    std::unique_ptr<InGameScene> inGameScene;
    Scene* currentScene;
};

#endif
