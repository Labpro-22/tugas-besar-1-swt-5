#include "../../include/views/Flexbox.hpp"

void Flexbox::add(UIElement* element) {
    children.push_back(element);
}

void Flexbox::update() {
    for (auto& child : children) {
        child->update();
    }
}