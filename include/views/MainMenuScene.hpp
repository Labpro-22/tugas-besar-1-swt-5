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
    Button registerButton;
    Button startGameButton;
    Button cancelButton;
    Button plusButton;
    Button minusButton;
    Button confirmLoadButton;
    Button cancelLoadButton;
    Button confirmRegisterButton;
    Button cancelRegisterButton;

    Popup setupModalPopup;
    Popup loadModalPopup;
    Popup registerModalPopup;

    std::vector<TextField> playerFields;
    std::vector<TextField> passwordFields;
    TextField configPathField;
    TextField loadPathField;
    TextField usernameField;
    TextField passwordField;
    std::string formError;
    std::string loadError;
    std::string registerError;
    int   playerCount;
    float sceneTime;
    std::string errorMsg;

    void layoutButtons(Rectangle sr);
    void drawBackground(Rectangle sr);
    void drawHero(Rectangle sr);
    void drawFeatureCards(Rectangle sr);
    void drawSetupModal(Rectangle sr);
    void drawLoadModal(Rectangle sr);
    void drawRegisterModal(Rectangle sr);
    void onStartGame();
    void onLoadGame();
    void onRegister();
};

#endif
