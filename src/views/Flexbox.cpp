#include "../../include/views/Flexbox.hpp"

Flexbox::Flexbox() {
    boundingBox = {0.0f, 0.0f, 0.0f, 0.0f};
}

Flexbox::Flexbox(Rectangle bound) {
    boundingBox = bound;
}

void Flexbox::add(UIElement* element) {
    children.push_back(element);
}

void Flexbox::clear() {
    children.clear();
}

void Flexbox::draw() {
    for (UIElement* child : children) {
        if (child != nullptr) {
            child->draw();
        }
    }
}

void Flexbox::update() {
    for (UIElement* child : children) {
        if (child != nullptr) {
            child->update();
        }
    }
}
