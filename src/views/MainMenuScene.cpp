#include "../../include/views/MainMenuScene.hpp"

void MainMenuScene::draw() {
    root.setBoundary(boundingBox);
    root.layout();
    root.draw();
    if (currentPopup) {
        currentPopup->draw();
    }
}

void MainMenuScene::update() {
    root.update();
    if (currentPopup) {
        currentPopup->update();
    }
}