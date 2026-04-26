#include "../../include/views/Button.hpp"
#include <utility>

namespace {
constexpr float kFontSize = 20.0f;
}

Button::Button() {
    preferredWidth = 180.0f;
    preferredHeight = 48.0f;
}

Button::Button(const std::string& textValue, Color background, Color foreground)
    : text(textValue), bgColor(background), textColor(foreground) {
    preferredWidth = 180.0f;
    preferredHeight = 48.0f;
}

void Button::setText(const std::string& value) {
    text = value;
}

const std::string& Button::getText() const {
    return text;
}

void Button::setOnClick(std::function<void()> callback) {
    onButtonClick = std::move(callback);
}

void Button::update() {
    if (disabled) {
        hovered = false;
        pressed = false;
        return;
    }

    hovered = CheckCollisionPointRec(GetMousePosition(), boundingBox);
    if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        pressed = true;
    }

    if (pressed && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        pressed = false;
        if (hovered && onButtonClick) {
            onButtonClick();
        }
    }

    if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        pressed = false;
    }
}

void Button::draw() {
    Color base = disabled ? Color{110, 120, 150, 110} : bgColor;
    Color fill = base;
    if (hovered && !disabled) {
        fill = ColorBrightness(base, 0.12f);
    }
    if (pressed && !disabled) {
        fill = ColorBrightness(base, -0.12f);
    }

    DrawRectangleRounded({boundingBox.x + 4, boundingBox.y + 6, boundingBox.width, boundingBox.height}, 0.28f, 8, Fade(BLACK, 0.12f));
    DrawRectangleRounded(boundingBox, 0.28f, 8, fill);
    DrawRectangleRoundedLinesEx(boundingBox, 0.28f, 8, 2.0f, Fade(WHITE, disabled ? 0.12f : 0.3f));

    const int fontSize = static_cast<int>(kFontSize);
    const int textWidth = MeasureText(text.c_str(), fontSize);
    const float textX = boundingBox.x + (boundingBox.width - static_cast<float>(textWidth)) * 0.5f;
    const float textY = boundingBox.y + (boundingBox.height - static_cast<float>(fontSize)) * 0.5f - 2.0f;
    DrawText(text.c_str(), static_cast<int>(textX), static_cast<int>(textY), fontSize, disabled ? Fade(textColor, 0.5f) : textColor);
}
