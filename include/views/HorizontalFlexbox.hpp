#ifndef HORIZONTAL_FLEXBOX_HPP
#define HORIZONTAL_FLEXBOX_HPP

#include "Flexbox.hpp"

class HorizontalFlexbox : public Flexbox {
public:
    HorizontalFlexbox();
    explicit HorizontalFlexbox(Rectangle bound);
    void layout() override;
};

#endif
