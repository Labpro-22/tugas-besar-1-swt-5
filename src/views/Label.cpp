#include "../../include/views/Label.hpp"

Label::Label() {
    preferredWidth = 120.0f;
    preferredHeight = 24.0f;
}

Label::Label(const std::string& value, Color tint, int fontSize)
    : text(value), color(tint), size(fontSize) {
    preferredWidth = static_cast<float>(MeasureText(text.c_str(), size));
    preferredHeight = static_cast<float>(size + 2);
}

void Label::setText(const std::string& value) {
    text = value;
    preferredWidth = static_cast<float>(MeasureText(text.c_str(), size));
}

void Label::setColor(Color value) {
    color = value;
}

void Label::setFontSize(int value) {
    size = value;
    preferredWidth = static_cast<float>(MeasureText(text.c_str(), size));
    preferredHeight = static_cast<float>(size + 2);
}

void Label::draw() {
    DrawText(text.c_str(), static_cast<int>(boundingBox.x), static_cast<int>(boundingBox.y), size, color);
}
