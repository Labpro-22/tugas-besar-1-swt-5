#include "../../include/views/VerticalFlexbox.hpp"

VerticalFlexbox::VerticalFlexbox() : Flexbox() {}

VerticalFlexbox::VerticalFlexbox(Rectangle bound) : Flexbox(bound) {}

void VerticalFlexbox::layout() {
    float cursorY = boundingBox.y + padding;
    for (UIElement* child : children) {
        if (child == nullptr) {
            continue;
        }

        float childX = boundingBox.x + padding;
        if (alignment == Alignment::CENTER) {
            childX = boundingBox.x + (boundingBox.width - child->preferredWidth) * 0.5f;
        } else if (alignment == Alignment::END) {
            childX = boundingBox.x + boundingBox.width - padding - child->preferredWidth;
        }

        child->setBoundary({childX, cursorY, child->preferredWidth, child->preferredHeight});
        cursorY += child->preferredHeight + spacing;
    }
}
