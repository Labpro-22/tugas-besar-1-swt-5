#ifndef POPUP_HPP
#define POPUP_HPP

#include <string>
#include "UIElement.hpp"

class Popup : public UIElement {
public:
    Popup();
    Popup(float width, float height);

    void setTitle(const std::string& value);
    void setVisible(bool value);
    bool isVisible() const;

    void draw() override;

private:
    std::string title;
    bool active = false;
};

#endif
