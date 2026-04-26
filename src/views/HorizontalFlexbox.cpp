#include "../../include/views/HorizontalFlexbox.hpp"

HorizontalFlexbox::HorizontalFlexbox() : Flexbox() {}

HorizontalFlexbox::HorizontalFlexbox(Rectangle bound) : Flexbox(bound) {}

void HorizontalFlexbox::layout() {
    float cursorX = boundingBox.x + padding;
    for (UIElement* child : children) {
        if (child == nullptr) {
            continue;
        }

        float childY = boundingBox.y + padding;
        if (alignment == Alignment::CENTER) {
            childY = boundingBox.y + (boundingBox.height - child->preferredHeight) * 0.5f;
        } else if (alignment == Alignment::END) {
            childY = boundingBox.y + boundingBox.height - padding - child->preferredHeight;
        }

        child->setBoundary({cursorX, childY, child->preferredWidth, child->preferredHeight});
        cursorX += child->preferredWidth + spacing;
    }
}
