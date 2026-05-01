#ifndef MAIN_MENU_SCENE_HPP
#define MAIN_MENU_SCENE_HPP

#include <vector>
#include <string>
#include "Button.hpp"
#include "Popup.hpp"
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

    Popup setupModalPopup;
    Popup loadModalPopup;

    std::vector<TextField> playerFields;
    TextField configPathField;
    TextField loadPathField;
    std::string formError;
    std::string loadError;
    int   playerCount;
    float sceneTime;
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
