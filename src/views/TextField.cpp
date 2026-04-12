#include "../../include/views/TextField.hpp"

TextField:: TextField(const std::string& text) : text(text), isActive(false), cursorPos(0) {
    preferredHeight = 30;
    preferredWidth = 200;
}

void TextField::draw() {
    Color bgColor = isActive ? LIGHTGRAY : GRAY;
    DrawRectangle(boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height, bgColor);
    DrawText(text.c_str(), boundingBox.x + 5, boundingBox.y + 5, 20, BLACK);
    if (isActive) {
        int cursorX = boundingBox.x + 5 + MeasureText(text.substr(0, cursorPos).c_str(), 20);
        DrawLine(cursorX, boundingBox.y + 5, cursorX, boundingBox.y + boundingBox.height - 5, BLACK);
    }
}

void TextField::update() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        isActive = CheckCollisionPointRec(GetMousePosition(), boundingBox);
    }
    if (isActive) {
        int key = GetKeyPressed();
        if (key > 0) {
            if (key == KEY_BACKSPACE && !text.empty() && cursorPos > 0) {
                text.erase(cursorPos - 1, 1);
                cursorPos--;
            } else if (key == KEY_DELETE && !text.empty() && cursorPos < text.size()) {
                text.erase(cursorPos, 1);
            } else if (key == KEY_LEFT && cursorPos > 0) {
                cursorPos--;
            } else if (key == KEY_RIGHT && cursorPos < text.size()) {
                cursorPos++;
            }
        }
        int charPressed = GetCharPressed();
        while (charPressed > 0) {
            text.insert(cursorPos, 1, static_cast<char>(charPressed));
            cursorPos++;
            charPressed = GetCharPressed();
        }
    }
}

std::string TextField::getContent() {
    return text;
}