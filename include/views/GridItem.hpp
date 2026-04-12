#ifndef GRID_ITEM_HPP
#define GRID_ITEM_HPP
#include "UIElement.hpp"

class GridItem : public UIElement {
friend class GridContainer;
public:
    GridItem(UIElement* item, int row, int col, int rowSpan, int colSpan);
    void draw() override;
    void update() override;
    void layout();

private:
    UIElement* item;
    int row;
    int col;
    int rowSpan;
    int colSpan;
};

#endif