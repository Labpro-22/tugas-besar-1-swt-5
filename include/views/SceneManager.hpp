#ifndef SCENEMANAGER_HPP
#define SCENEMANAGER_HPP
#include "Scene.hpp"

// Forward declarations to avoid circular dependencies
class AccountManager;
class GameManager; 

class SceneManager {
private:
    Scene* currentScene;
    AccountManager* accountManager;
    GameManager* gameManager;
    
public:
    SceneManager() : currentScene(), accountManager(nullptr), gameManager(nullptr) {}
    void setScene(Scene* newScene);
    void update();
    void draw();
};

#endif