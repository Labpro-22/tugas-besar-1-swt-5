#include "../../include/views/GridContainer.hpp"

GridContainer::GridContainer(int rowCount, int colCount) : rows(rowCount), cols(colCount) {}

void GridContainer::add(const GridItem& item) {
    items.push_back(item);
}

void GridContainer::draw() {
    for (GridItem& item : items) {
        item.draw();
    }
}

void GridContainer::update() {
    for (GridItem& item : items) {
        item.update();
    }
}

void GridContainer::layout() {
    if (rows <= 0 || cols <= 0) {
        return;
    }

    const float cellWidth = boundingBox.width / static_cast<float>(cols);
    const float cellHeight = boundingBox.height / static_cast<float>(rows);

    for (GridItem& item : items) {
        Rectangle bound{
            boundingBox.x + cellWidth * static_cast<float>(item.col),
            boundingBox.y + cellHeight * static_cast<float>(item.row),
            cellWidth * static_cast<float>(item.colSpan),
            cellHeight * static_cast<float>(item.rowSpan)
        };
        item.setBoundary(bound);
        item.layout();
    }
}
