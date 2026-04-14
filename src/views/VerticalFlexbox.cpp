#include "../../include/views/VerticalFlexbox.hpp"

VerticalFlexbox::VerticalFlexbox(Rectangle bound) {
    spacing= 10.0f;
    padding= 10.0f;
    alignment= Alignment::START;
    boundingBox= bound;
}

void VerticalFlexbox::layout() {
    float yOffset = boundingBox.y + padding;
    for (auto& child : children) {
        float xOffset = boundingBox.x + padding;
        if (alignment == Alignment::CENTER) {
            xOffset += (boundingBox.width - padding * 2 - child->preferredWidth) / 2;
        } else if (alignment == Alignment::END) {
            xOffset += boundingBox.width - padding * 2 - child->preferredWidth;
        }
        child->setBoundary({xOffset, yOffset, child->preferredWidth, child->preferredHeight});
        yOffset += child->preferredHeight + spacing;
    }
}