#ifndef TEXT_FIELD_HPP
#define TEXT_FIELD_HPP

#include <string>
#include "UIElement.hpp"

class TextField : public UIElement {
public:
    TextField();
    explicit TextField(const std::string& placeholder);

    void setPlaceholder(const std::string& value);
    void setContent(const std::string& value);
    const std::string& getContent() const;
    bool hasFocus() const;

    void update() override;
    void draw() override;

private:
    std::string placeholder;
    std::string text;
    bool isActive = false;
    int maxLength = 18;
};

#endif
