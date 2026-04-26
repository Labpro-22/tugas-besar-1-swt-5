#ifndef GRID_CONTAINER_HPP
#define GRID_CONTAINER_HPP

#include <vector>
#include "GridItem.hpp"

class GridContainer : public UIElement {
public:
    GridContainer(int rows = 1, int cols = 1);
    void add(const GridItem& item);
    void draw() override;
    void update() override;
    void layout();

private:
    int rows = 1;
    int cols = 1;
    std::vector<GridItem> items;
};

#endif
