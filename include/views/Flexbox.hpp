#ifndef FLEXBOX_HPP
#define FLEXBOX_HPP
#include <vector>
#include "UIElement.hpp"

enum class Alignment {
    START,
    CENTER,
    END,
};

class Flexbox : public UIElement {
protected:
    std::vector<UIElement*> children;

public:
    Flexbox() {
        boundingBox = {0, 0, 800, 600};
    }
    Flexbox(Rectangle bound) {
        boundingBox = bound;
    }
    float spacing;
    float padding;
    Alignment alignment;
    void add(UIElement* element);
    virtual void layout() = 0;
    virtual void draw();
    virtual void update();  
};

#endif