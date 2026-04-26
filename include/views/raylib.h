#ifndef NIMONSPOLI_RAYLIB_COMPAT_HPP
#define NIMONSPOLI_RAYLIB_COMPAT_HPP

#if defined(NIMONSPOLI_USE_REAL_RAYLIB) && defined(__has_include_next)
#if __has_include_next(<raylib.h>)
#include_next <raylib.h>
#define NIMONSPOLI_HAS_REAL_RAYLIB 1
#endif
#endif

#ifndef NIMONSPOLI_HAS_REAL_RAYLIB

#include <cmath>
#include <cstdint>
#include <ctime>

typedef struct Vector2 {
    float x;
    float y;
} Vector2;

typedef struct Rectangle {
    float x;
    float y;
    float width;
    float height;
} Rectangle;

typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

typedef struct Texture2D {
    unsigned int id;
    int width;
    int height;
    int mipmaps;
    int format;
} Texture2D;

static constexpr int FLAG_WINDOW_RESIZABLE = 4;
static constexpr int FLAG_MSAA_4X_HINT = 32;
static constexpr int FLAG_VSYNC_HINT = 64;

static constexpr int MOUSE_LEFT_BUTTON = 0;
static constexpr int KEY_ESCAPE = 256;
static constexpr int KEY_BACKSPACE = 259;

static constexpr Color WHITE{255, 255, 255, 255};
static constexpr Color BLACK{0, 0, 0, 255};
static constexpr Color DARKGREEN{0, 117, 44, 255};
static constexpr Color DARKGRAY{80, 80, 80, 255};
static constexpr Color LIGHTGRAY{200, 200, 200, 255};

namespace raylib_stub {
inline int& screenWidth() {
    static int value = 1440;
    return value;
}

inline int& screenHeight() {
    static int value = 980;
    return value;
}

inline bool& windowOpen() {
    static bool value = false;
    return value;
}

inline double startTime() {
    static const double value = static_cast<double>(std::clock()) / static_cast<double>(CLOCKS_PER_SEC);
    return value;
}
}

inline void SetConfigFlags(unsigned int) {}
inline void InitWindow(int width, int height, const char*) {
    raylib_stub::screenWidth() = width;
    raylib_stub::screenHeight() = height;
    raylib_stub::windowOpen() = true;
}
inline void SetTargetFPS(int) {}
inline bool WindowShouldClose() {
    static bool first = true;
    if (first) {
        first = false;
        return false;
    }
    return true;
}
inline void CloseWindow() {
    raylib_stub::windowOpen() = false;
}
inline void BeginDrawing() {}
inline void EndDrawing() {}
inline void ClearBackground(Color) {}
inline float GetFrameTime() { return 1.0f / 60.0f; }
inline int GetScreenWidth() { return raylib_stub::screenWidth(); }
inline int GetScreenHeight() { return raylib_stub::screenHeight(); }
inline double GetTime() {
    return static_cast<double>(std::clock()) / static_cast<double>(CLOCKS_PER_SEC) - raylib_stub::startTime();
}

inline Vector2 GetMousePosition() { return Vector2{0.0f, 0.0f}; }
inline bool IsMouseButtonPressed(int) { return false; }
inline bool IsMouseButtonReleased(int) { return false; }
inline bool IsMouseButtonDown(int) { return false; }
inline bool IsKeyPressed(int) { return false; }
inline int GetCharPressed() { return 0; }

inline bool CheckCollisionPointRec(Vector2 point, Rectangle rec) {
    return point.x >= rec.x && point.x <= rec.x + rec.width && point.y >= rec.y && point.y <= rec.y + rec.height;
}

inline Color Fade(Color color, float alpha) {
    Color copy = color;
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    copy.a = static_cast<unsigned char>(255.0f * alpha);
    return copy;
}

inline Color ColorBrightness(Color color, float factor) {
    auto adjust = [factor](unsigned char c) -> unsigned char {
        float value = static_cast<float>(c);
        if (factor >= 0.0f) {
            value += (255.0f - value) * factor;
        } else {
            value += value * factor;
        }
        if (value < 0.0f) value = 0.0f;
        if (value > 255.0f) value = 255.0f;
        return static_cast<unsigned char>(value);
    };

    return Color{adjust(color.r), adjust(color.g), adjust(color.b), color.a};
}

inline int MeasureText(const char* text, int fontSize) {
    int count = 0;
    if (text != nullptr) {
        while (text[count] != '\0') {
            ++count;
        }
    }
    return count * fontSize / 2;
}

inline Texture2D LoadTexture(const char*) {
    return Texture2D{1, 0, 0, 1, 0};
}
inline void UnloadTexture(Texture2D) {}
inline void DrawTextureEx(Texture2D, Vector2, float, float, Color) {}

inline void DrawText(const char*, int, int, int, Color) {}
inline void DrawRectangle(int, int, int, int, Color) {}
inline void DrawRectangleRec(Rectangle, Color) {}
inline void DrawRectangleLinesEx(Rectangle, float, Color) {}
inline void DrawRectangleRounded(Rectangle, float, int, Color) {}
inline void DrawRectangleRoundedLinesEx(Rectangle, float, int, float, Color) {}
inline void DrawRectangleGradientV(int, int, int, int, Color, Color) {}
inline void DrawRectanglePro(Rectangle, Vector2, float, Color) {}
inline void DrawCircle(int, int, float, Color) {}
inline void DrawCircleLines(int, int, float, Color) {}
inline void DrawLineEx(Vector2, Vector2, float, Color) {}

#endif

#endif
