#include "../../include/views/Popup.hpp"
#include "raylib.h"
#include <algorithm>

Popup::Popup() {
    boundingBox = {200.0f, 150.0f, 400.0f, 300.0f};
}

Popup::Popup(float width, float height) {
    boundingBox = {0.0f, 0.0f, width, height};
}

void Popup::setTitle(const std::string& value) {
    title = value;
}

void Popup::setVisible(bool value) {
    active = value;
    targetVisibility = value ? 1.0f : 0.0f;
}

bool Popup::isVisible() const {
    return active;
}

void Popup::update() {
    // Animate visibility
    float frameTime = GetFrameTime();
    float step = animationSpeed * frameTime;
    
    if (visibility < targetVisibility) {
        visibility = std::min(visibility + step, targetVisibility);
    } else if (visibility > targetVisibility) {
        visibility = std::max(visibility - step, targetVisibility);
    }
}

void Popup::setShowOverlay(bool show) {
    showOverlay = show;
}

bool Popup::getShowOverlay() const {
    return showOverlay;
}

float Popup::getVisibility() const {
    return visibility;
}

void Popup::setAnimationSpeed(float speed) {
    animationSpeed = speed;
}

void Popup::centerOnScreen() {
    boundingBox.x = (GetScreenWidth() - boundingBox.width) * 0.5f;
    boundingBox.y = (GetScreenHeight() - boundingBox.height) * 0.5f;
}

Rectangle Popup::getContentArea() const {
    return {
        boundingBox.x + 26.0f,
        boundingBox.y + 60.0f,
        boundingBox.width - 52.0f,
        boundingBox.height - 100.0f
    };
}

void Popup::drawOverlay() {
    if (showOverlay && visibility > 0.01f) {
        DrawRectangle(
            0,
            0,
            GetScreenWidth(),
            GetScreenHeight(),
            Fade({50, 80, 20, 255}, 0.35f * visibility)
        );
    }
}

void Popup::drawModalBackground() {
    if (visibility < 0.01f) {
        return;
    }

    // Shadow
    DrawRectangleRounded(
        {boundingBox.x + 6, boundingBox.y + 10, boundingBox.width, boundingBox.height},
        0.1f,
        12,
        Fade({50, 80, 20, 255}, 0.1f * visibility)
    );

    // Main box
    DrawRectangleRounded(
        boundingBox,
        0.1f,
        12,
        Fade({250, 255, 235, 255}, visibility)
    );

    // Border
    DrawRectangleRoundedLinesEx(
        boundingBox,
        0.1f,
        12,
        2.5f,
        Fade({180, 210, 120, 200}, visibility)
    );

    // Title
    if (!title.empty()) {
        DrawText(
            title.c_str(),
            static_cast<int>(boundingBox.x + 26),
            static_cast<int>(boundingBox.y + 24),
            34,
            Fade({50, 80, 20, 255}, visibility)
        );
    }
}

void Popup::draw() {
    drawOverlay();
    drawModalBackground();
}
