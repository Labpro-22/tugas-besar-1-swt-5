#include "../../include/views/Nimonspoli.hpp"
#include "raylib.h"
#include "../../include/core/RealGameFacade.hpp"

Nimonspoli::~Nimonspoli() = default;

void Nimonspoli::setup() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1440, 980, "Nimonspoli");
    SetTargetFPS(60);

    gameFacade = std::make_unique<RealGameFacade>();
    sceneManager.initialize(gameFacade.get());
    sceneManager.setScene(SceneType::MainMenu);
}

void Nimonspoli::start() {
    while (!WindowShouldClose()) {
        update();
        BeginDrawing();
        ClearBackground({245, 250, 235, 255});
        draw();
        EndDrawing();
    }
    CloseWindow();
}

void Nimonspoli::update() {
    if (gameFacade) gameFacade->tick(GetFrameTime());
    sceneManager.update();
}

void Nimonspoli::draw() {
    sceneManager.draw();
}
