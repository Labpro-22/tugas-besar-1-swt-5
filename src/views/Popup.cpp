#include "../../include/views/Popup.hpp"

Popup::Popup(float width, float height) {
    isActive = false; 
    float x = (800 - width) / 2;
    float y = (600 - height) / 2;
    container = VerticalFlexbox({x, y, width, height});
}

void Popup::draw() {
    if (isActive) {
        container.layout();
        container.draw();
    }
}

void Popup::update() {
    if (isActive) {
        container.update();
    }
}

void Popup::add(UIElement* element) {
    container.add(element);
}