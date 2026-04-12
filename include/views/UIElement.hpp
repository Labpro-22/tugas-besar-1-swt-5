#ifndef UIELEMENT_HPP
#define UIELEMENT_HPP

#include "raylib.h"

class UIElement {
public:
    float preferredWidth;
    float preferredHeight;

    virtual void draw() = 0;
    virtual void update() {}
    virtual void setBoundary(Rectangle newBound);
    virtual Rectangle getBoundary();
    virtual ~UIElement() = default;

protected:
    Rectangle boundingBox;
};

#endif