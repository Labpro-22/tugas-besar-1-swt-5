#include "../../include/views/Popup.hpp"

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
}

bool Popup::isVisible() const {
    return active;
}

void Popup::draw() {
    if (!active) {
        return;
    }

    DrawRectangleRounded({boundingBox.x + 4.0f, boundingBox.y + 6.0f, boundingBox.width, boundingBox.height}, 0.18f, 8, Fade(BLACK, 0.2f));
    DrawRectangleRounded(boundingBox, 0.18f, 8, Color{26, 32, 64, 250});
    DrawRectangleRoundedLinesEx(boundingBox, 0.18f, 8, 2.0f, Fade(WHITE, 0.18f));
    if (!title.empty()) {
        DrawText(title.c_str(), static_cast<int>(boundingBox.x + 20.0f), static_cast<int>(boundingBox.y + 18.0f), 26, WHITE);
    }
}
