#include "../../include/views/Nimonspoli.hpp"

void Nimonspoli::setup() {
    InitWindow(800, 600, "Nimonspoli");
    SetTargetFPS(60);
    // set scene awal nanti setelah GameManager selesai
}

void Nimonspoli::start() {
    while (!WindowShouldClose()) {
        update();
        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw();
        EndDrawing();
    }
}

void Nimonspoli::update() {
    sceneManager.update();
}

void Nimonspoli::draw() {
    sceneManager.draw();
}