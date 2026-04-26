#ifndef NIMONSPOLI_HPP
#define NIMONSPOLI_HPP

#include <memory>
#include "SceneManager.hpp"
#include "../core/IGameFacade.hpp"

class Nimonspoli {
public:
    ~Nimonspoli();
    void setup();
    void start();
    void update();
    void draw();

private:
    std::unique_ptr<IGameFacade> gameFacade;
    SceneManager sceneManager;
};

#endif
