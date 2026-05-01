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
    void update() override;
    void draw() override;

    // Modal animation and state
    void setShowOverlay(bool show);
    bool getShowOverlay() const;
    float getVisibility() const;
    void setAnimationSpeed(float speed);
    void centerOnScreen();

    // Get padding and inner content area
    Rectangle getContentArea() const;

private:
    std::string title;
    bool active = false;
    bool showOverlay = true;
    float visibility = 0.0f;  // 0.0 to 1.0
    float targetVisibility = 0.0f;
    float animationSpeed = 9.0f;  // units per second

    void drawOverlay();
    void drawModalBackground();
};

#endif
