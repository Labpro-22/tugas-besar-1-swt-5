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
    Flexbox();
    explicit Flexbox(Rectangle bound);

    float spacing = 10.0f;
    float padding = 10.0f;
    Alignment alignment = Alignment::START;

    void add(UIElement* element);
    void clear();
    void draw() override;
    void update() override;
    virtual void layout() = 0;
};

#endif
