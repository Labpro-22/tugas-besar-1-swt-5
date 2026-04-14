#ifndef NIMONSPOLI_HPP
#define NIMONSPOLI_HPP
#include "SceneManager.hpp"
// #include "../../include/core/GameManager.hpp"    // uncomment saat siap
// #include "../../include/core/AccountManager.hpp" // uncomment saat siap

class GameManager;
class AccountManager;

class Nimonspoli {
private:
    SceneManager sceneManager;
    // GameManager gameManager;    // uncomment saat siap
    // AccountManager accountManager; // uncomment saat siap

public:
    void setup();
    void start();
    void update();
    void draw();
};

#endif