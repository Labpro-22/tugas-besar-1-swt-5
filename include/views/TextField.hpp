#ifndef TEXTFIELD_HPP
#define TEXTFIELD_HPP
#include <string>
#include "UIElement.hpp"

class TextField : public UIElement {
public:
    TextField(const std::string& text);
    void draw() override;
    void update() override;
    std::string getContent();

private:
    std::string text;
    bool isActive;
    int cursorPos;
};

#endif