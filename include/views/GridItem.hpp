#ifndef GRID_ITEM_HPP
#define GRID_ITEM_HPP

#include "UIElement.hpp"

class GridItem : public UIElement {
    friend class GridContainer;

public:
    GridItem(UIElement* item = nullptr, int row = 0, int col = 0, int rowSpan = 1, int colSpan = 1);
    void draw() override;
    void update() override;
    void layout();

private:
    UIElement* item = nullptr;
    int row = 0;
    int col = 0;
    int rowSpan = 1;
    int colSpan = 1;
};

#endif
