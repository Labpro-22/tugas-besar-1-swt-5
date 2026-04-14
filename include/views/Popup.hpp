#ifndef POPUP_HPP
#define POPUP_HPP
#include "VerticalFlexbox.hpp"
#include "UIElement.hpp"
class Popup {//Ada perubahan, Popup jadi anak dari UIElement biar bisa pake boundingBox langsung
private:
    VerticalFlexbox container;

public:
    Popup(float width, float height);
    bool isActive;
    void draw();
    void update();
    void add(UIElement* element);
};

#endif