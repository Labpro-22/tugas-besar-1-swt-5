#ifndef VERTICAL_FLEXBOX_HPP
#define VERTICAL_FLEXBOX_HPP

#include "Flexbox.hpp"

class VerticalFlexbox : public Flexbox {
public:
    VerticalFlexbox();
    explicit VerticalFlexbox(Rectangle bound);
    void layout() override;
};

#endif
