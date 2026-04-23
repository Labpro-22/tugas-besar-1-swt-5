#ifndef PROPERTYCARDWIDGET_HPP
#define PROPERTYCARDWIDGET_HPP
#include <string>
#include "UIElement.hpp"

class PropertyCardWidget : public UIElement {
private:
    Texture2D templateTexture;
    std::string cityName;
    std::string priceText;
    bool textureLoaded;

public:
    PropertyCardWidget(const std::string& templatePath,
                       const std::string& cityName,
                       int price);
    ~PropertyCardWidget();
    void draw() override;
};

#endif