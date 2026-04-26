#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <functional>
#include <string>
#include "UIElement.hpp"

class Button : public UIElement {
public:
    Button();
    Button(const std::string& text, Color bgColor, Color textColor);

    void setText(const std::string& value);
    const std::string& getText() const;
    void setOnClick(std::function<void()> callback);

    void update() override;
    void draw() override;

    bool disabled = false;

private:
    std::string text;
    Color bgColor{70, 92, 236, 255};
    Color textColor{245, 247, 255, 255};
    std::function<void()> onButtonClick;
    bool hovered = false;
    bool pressed = false;
};

#endif
