#include "../../include/views/Nimonspoli.hpp"

#include "raylib.h"
#include "../../include/coredummy/IGameFacade.hpp"
#include "../../include/coredummy/MockGameFacade.hpp"

Nimonspoli::~Nimonspoli() = default;

void Nimonspoli::setup() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(1440, 980, "Nimonspoli Raylib GUI");
    SetTargetFPS(60);

    gameFacade = std::make_unique<MockGameFacade>();
    sceneManager.initialize(gameFacade.get());
    sceneManager.setScene(SceneType::MainMenu);
}

void Nimonspoli::start() {
    while (!WindowShouldClose()) {
        update();
        BeginDrawing();
        ClearBackground(BLACK);
        draw();
        EndDrawing();
    }
    CloseWindow();
}

void Nimonspoli::update() {
    if (gameFacade) {
        gameFacade->tick(GetFrameTime());
    }
    sceneManager.update();
}

void Nimonspoli::draw() {
    sceneManager.draw();
}
