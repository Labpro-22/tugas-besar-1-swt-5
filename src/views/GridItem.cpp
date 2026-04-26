#include "../../include/views/GridItem.hpp"

GridItem::GridItem(UIElement* value, int rowValue, int colValue, int rowSpanValue, int colSpanValue)
    : item(value), row(rowValue), col(colValue), rowSpan(rowSpanValue), colSpan(colSpanValue) {}

void GridItem::draw() {
    if (item != nullptr) {
        item->draw();
    }
}

void GridItem::update() {
    if (item != nullptr) {
        item->update();
    }
}

void GridItem::layout() {
    if (item != nullptr) {
        item->setBoundary(boundingBox);
    }
}
