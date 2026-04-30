#ifndef IN_GAME_SCENE_HPP
#define IN_GAME_SCENE_HPP

#include <string>
#include <vector>
#include "Button.hpp"
#include "Scene.hpp"

class InGameScene : public Scene {
public:
    InGameScene(SceneManager* sm, GameManager* gm, AccountManager* am);
    void update() override;
    void draw()   override;
    void onEnter() override;
private:
    std::vector<Button> actionButtons;
    Button closeOverlayBtn;
    Button backToMenuBtn;
    std::vector<Rectangle> tileRects;
    std::vector<Vector2> tokenPos;
    std::vector<float>   tokenPhase;
    float sceneTime;
    int selectedTile;

    // Simple overlay
    bool overlayOpen;
    std::string overlayTitle;
    std::vector<std::string> overlayLines;
    std::string overlayFooter;
    float overlayVis;

    void layoutUi(Rectangle sr, Rectangle& br, Rectangle& sb);
    Rectangle getTileRect(const Rectangle& br, int idx) const;
    Vector2   getTileCenter(const Rectangle& br, int idx) const;
    void updateAnimations(const Rectangle& br);
    void drawBackground(Rectangle sr);
    void drawHeader(Rectangle sr);
    void drawBoard(const Rectangle& br);
    void drawCenterPanel(const Rectangle& br);
    void drawSidebar(const Rectangle& sb);
    void drawOverlay(Rectangle sr);
    void showOverlay(
        const std::string& title,
        const std::vector<std::string>& lines,
        const std::string& footer = ""
    );
};

#endif
