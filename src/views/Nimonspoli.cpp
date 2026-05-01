#include "../../include/views/Nimonspoli.hpp"
#include "../../include/data-layer/AccountDataManager.hpp"
#include "raylib.h"

void Nimonspoli::setup() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1440, 980, "Nimonspoli");
    SetTargetFPS(60);
    AccountDataManager accountLoader("./data/account.txt");
    accountLoader.loadData(accountManager);
    sceneManager.initialize(&gameManager, &accountManager);
    sceneManager.setScene(SceneType::MainMenu);
}

void Nimonspoli::start() {
    while (!WindowShouldClose()) {
        update();
        BeginDrawing();
        ClearBackground({240, 248, 225, 255});
        draw();
        EndDrawing();
    }
    CloseWindow();
}

void Nimonspoli::update() { sceneManager.update(); }
void Nimonspoli::draw()   { sceneManager.draw(); }
Nimonspoli::~Nimonspoli() {
    AccountDataManager accountLoader("./data/account.txt");
    accountLoader.saveData(accountManager);
}