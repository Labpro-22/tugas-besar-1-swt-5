#include "../../include/views/PropertyCardWidget.hpp"

PropertyCardWidget::PropertyCardWidget(const std::string& templatePath,
                                       const std::string& cityName,
                                       int price,
                                       Color accentColor)
    : cityName(cityName),
      priceText("M" + std::to_string(price)),
      textureLoaded(false),
      accentColor(accentColor) {
    preferredWidth = 150;
    preferredHeight = 200;
    templateTexture = LoadTexture(templatePath.c_str());
    textureLoaded = (templateTexture.id != 0);
}

PropertyCardWidget::PropertyCardWidget(const std::string& cityName,
                                       int price,
                                       Color accentColor)
    : cityName(cityName),
      priceText("M" + std::to_string(price)),
      textureLoaded(false),
      accentColor(accentColor) {
    preferredWidth = 150;
    preferredHeight = 200;
    templateTexture.id = 0;
}

PropertyCardWidget::~PropertyCardWidget() {
    if (textureLoaded) {
        UnloadTexture(templateTexture);
    }
}

void PropertyCardWidget::draw() {
    if (textureLoaded) {
        // Gambar template PNG yang di-load
        DrawTextureEx(templateTexture,
                      {boundingBox.x, boundingBox.y},
                      0.0f, 1.0f, WHITE);
    } else {
        // Fallback: gambar rectangle dengan accent color
        DrawRectangleRec(boundingBox, WHITE);
        DrawRectangleLinesEx(boundingBox, 2, DARKGRAY);
        // Area accent color di atas
        Rectangle accentArea = {
            boundingBox.x + 5,
            boundingBox.y + 5,
            boundingBox.width - 10,
            30
        };
        DrawRectangleRec(accentArea, accentColor);
        // Area gambar placeholder
        Rectangle imageArea = {
            boundingBox.x + 10,
            boundingBox.y + 45,
            boundingBox.width - 20,
            90
        };
        DrawRectangleLinesEx(imageArea, 1, LIGHTGRAY);
        DrawText("(image)",
                 imageArea.x + imageArea.width/2 - 25,
                 imageArea.y + imageArea.height/2 - 5,
                 10, LIGHTGRAY);
    }

    // nama kota - font besar
    int nameWidth = MeasureText(cityName.c_str(), 16);
    DrawText(cityName.c_str(),
             boundingBox.x + (boundingBox.width - nameWidth) / 2,
             boundingBox.y + 145,
             16, BLACK);

    // harga - font kecil
    int priceWidth = MeasureText(priceText.c_str(), 14);
    DrawText(priceText.c_str(),
             boundingBox.x + (boundingBox.width - priceWidth) / 2,
             boundingBox.y + 170,
             14, DARKGRAY);
}
