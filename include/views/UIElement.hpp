#ifndef UI_ELEMENT_HPP
#define UI_ELEMENT_HPP

#include "raylib.h"

class UIElement {
public:
    virtual ~UIElement() = default;

    float preferredWidth = 0.0f;
    float preferredHeight = 0.0f;

    virtual void draw() = 0;
    virtual void update() {}

    virtual void setBoundary(Rectangle newBound) { boundingBox = newBound; }
    virtual Rectangle getBoundary() const { return boundingBox; }

protected:
    Rectangle boundingBox{0.0f, 0.0f, 0.0f, 0.0f};
};

#endif
