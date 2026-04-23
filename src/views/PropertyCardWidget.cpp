#include "../../include/views/PropertyCardWidget.hpp"

PropertyCardWidget::PropertyCardWidget(
    const std::string& templatePath,
    const std::string& cityName,
    int price)
    : cityName(cityName),
      priceText("M" + std::to_string(price)),
      textureLoaded(false) {

    preferredWidth = 150;
    preferredHeight = 200;

    templateTexture = LoadTexture(templatePath.c_str());
    textureLoaded = (templateTexture.id != 0);
}

PropertyCardWidget::~PropertyCardWidget() {
    if (textureLoaded) {
        UnloadTexture(templateTexture);
    }
}

void PropertyCardWidget::draw() {
    if (textureLoaded) {
        DrawTextureEx(
            templateTexture,
            {boundingBox.x, boundingBox.y},
            0.0f,
            1.0f,
            WHITE
        );
    } else {
        DrawRectangle(boundingBox.x, boundingBox.y, 150, 200, LIGHTGRAY);
    }

    // nama kota — font besar, di bawah
    DrawText(cityName.c_str(),
             boundingBox.x + 8,
             boundingBox.y + 145,
             16, BLACK);

    // harga — font lebih kecil
    DrawText(priceText.c_str(),
             boundingBox.x + 8,
             boundingBox.y + 168,
             12, DARKGRAY);
}