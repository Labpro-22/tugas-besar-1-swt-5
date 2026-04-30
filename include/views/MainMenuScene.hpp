#ifndef MAIN_MENU_SCENE_HPP
#define MAIN_MENU_SCENE_HPP

#include <vector>
#include <string>
#include "Button.hpp"
#include "Scene.hpp"
#include "TextField.hpp"

class MainMenuScene : public Scene {
public:
    MainMenuScene(SceneManager* sm, GameManager* gm, AccountManager* am);
    void update() override;
    void draw()   override;
    void onEnter() override;
private:
    Button newGameButton;
    Button loadGameButton;
    Button quitButton;
    Button startGameButton;
    Button cancelButton;
    Button plusButton;
    Button minusButton;
    Button confirmLoadButton;
    Button cancelLoadButton;

    std::vector<TextField> playerFields;
    bool  showSetupModal;
    TextField configPathField;
    TextField loadPathField;
    std::string formError;
    std::string loadError;
    bool  showNewGameModal;
    bool  showLoadGameModal;
    int   playerCount;
    float sceneTime;
    float modalVisibility;
    float loadModalVisibility;
    std::string errorMsg;

    void layoutButtons(Rectangle sr);
    void drawBackground(Rectangle sr);
    void drawHero(Rectangle sr);
    void drawFeatureCards(Rectangle sr);
    void drawSetupModal(Rectangle sr);
    void drawLoadModal(Rectangle sr);
    void onStartGame();
    void onLoadGame();
};

#endif
