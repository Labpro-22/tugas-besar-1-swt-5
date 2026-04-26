#include "../../include/views/TextField.hpp"

TextField::TextField() : TextField("Masukkan teks") {}

TextField::TextField(const std::string& placeholderValue) : placeholder(placeholderValue) {
    preferredWidth = 340.0f;
    preferredHeight = 52.0f;
}

void TextField::setPlaceholder(const std::string& value) {
    placeholder = value;
}

void TextField::setContent(const std::string& value) {
    text = value.substr(0, static_cast<std::size_t>(maxLength));
}

const std::string& TextField::getContent() const {
    return text;
}

bool TextField::hasFocus() const {
    return isActive;
}

void TextField::update() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        isActive = CheckCollisionPointRec(GetMousePosition(), boundingBox);
    }

    if (!isActive) {
        return;
    }

    int ch = GetCharPressed();
    while (ch > 0) {
        if (ch >= 32 && ch <= 126 && static_cast<int>(text.size()) < maxLength) {
            text.push_back(static_cast<char>(ch));
        }
        ch = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !text.empty()) {
        text.pop_back();
    }
}

void TextField::draw() {
    const Color panel = isActive ? Color{34, 43, 88, 255} : Color{23, 31, 67, 255};
    DrawRectangleRounded({boundingBox.x + 3, boundingBox.y + 4, boundingBox.width, boundingBox.height}, 0.22f, 8, Fade(BLACK, 0.18f));
    DrawRectangleRounded(boundingBox, 0.22f, 8, panel);
    DrawRectangleRoundedLinesEx(boundingBox, 0.22f, 8, 2.0f, isActive ? Color{103, 177, 255, 255} : Fade(WHITE, 0.18f));

    const bool empty = text.empty();
    const std::string& shown = empty ? placeholder : text;
    const Color tint = empty ? Color{150, 160, 205, 255} : Color{242, 245, 255, 255};
    DrawText(shown.c_str(), static_cast<int>(boundingBox.x + 16.0f), static_cast<int>(boundingBox.y + 15.0f), 22, tint);

    if (isActive && (static_cast<int>(GetTime() * 2.0) % 2 == 0)) {
        const int width = MeasureText(text.c_str(), 22);
        const float lineX = boundingBox.x + 16.0f + static_cast<float>(width) + 2.0f;
        DrawLineEx({lineX, boundingBox.y + 14.0f}, {lineX, boundingBox.y + boundingBox.height - 14.0f}, 2.0f, WHITE);
    }
}
