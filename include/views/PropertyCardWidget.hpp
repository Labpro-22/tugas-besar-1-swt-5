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
    Color accentColor;

public:
    // Constructor pakai texture dari file
    PropertyCardWidget(const std::string& templatePath,
                       const std::string& cityName,
                       int price,
                       Color accentColor);

    // Constructor tanpa texture (dummy/fallback, gambar pakai rectangle)
    PropertyCardWidget(const std::string& cityName,
                       int price,
                       Color accentColor);

    ~PropertyCardWidget();

    void draw() override;
};

#endif
