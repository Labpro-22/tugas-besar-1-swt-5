#ifndef MAIN_MENU_SCENE_HPP
#define MAIN_MENU_SCENE_HPP

#include <vector>
#include "Button.hpp"
#include "Scene.hpp"
#include "TextField.hpp"

class MainMenuScene : public Scene {
public:
    MainMenuScene(SceneManager* manager, IGameFacade* facade);

    void update() override;
    void draw()   override;
    void onEnter() override;

private:
    Button newGameButton;
    Button loadDemoButton;
    Button quitButton;
    Button startGameButton;
    Button cancelButton;
    Button plusButton;
    Button minusButton;

    std::vector<TextField> playerFields;
    bool  showNewGameModal;
    int   playerCount;
    float sceneTime;
    float modalVisibility;

    void layoutButtons(Rectangle screenRect);
    void drawBackground(Rectangle screenRect);
    void drawHero(Rectangle screenRect);
    void drawFeatureCards(Rectangle screenRect);
    void drawNewGameModal(Rectangle screenRect);
};

#endif
