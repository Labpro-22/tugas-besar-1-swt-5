#include "../../include/views/UIElement.hpp"

void UIElement::setBoundary(Rectangle newBound) {
    boundingBox = newBound;
}

Rectangle UIElement::getBoundary() {
    return boundingBox;
}

