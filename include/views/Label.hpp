#ifndef LABEL_HPP
#define LABEL_HPP

#include <string>
#include "UIElement.hpp"

class Label : public UIElement {
public:
    Label();
    Label(const std::string& text, Color color, int size);

    void setText(const std::string& value);
    void setColor(Color value);
    void setFontSize(int value);

    void draw() override;

private:
    std::string text;
    Color color{245, 247, 255, 255};
    int size = 20;
};

#endif
