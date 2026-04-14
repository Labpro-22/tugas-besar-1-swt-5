#ifndef SCENE_HPP
#define SCENE_HPP
#include <map>
#include <string>
#include "UIElement.hpp"
#include "VerticalFlexbox.hpp"
#include "Popup.hpp"

// Forward declarations to avoid circular dependencies
class GameManager;
class SceneManager;
class AccountManager;

class Scene { //Ada perubahan, Popup jadi anak dari UIElement biar bisa pake boundingBox langsung
protected:
    std::map<std::string, Popup> popups;
    VerticalFlexbox root;
    Popup* currentPopup;
    GameManager* gameManager;
    SceneManager* sceneManager;
    AccountManager* accountManager;

public:
    Scene() : currentPopup(nullptr), gameManager(nullptr), sceneManager(nullptr), accountManager(nullptr),
              root({0, 0, 800, 600}), popups({}) {

    }
    virtual void draw() = 0;
    virtual void update() = 0;
    void showPopup(const std::string& key);
    void collapsePopup();
    virtual ~Scene() = default;
};

#endif