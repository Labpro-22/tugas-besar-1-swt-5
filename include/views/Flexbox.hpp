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
    float spacing;
    float padding;
    Alignment alignment;
    void add(UIElement* element);
    virtual void layout() = 0;
    virtual void draw() = 0;
    virtual void update();  
};

#endif