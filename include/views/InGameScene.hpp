#ifndef IN_GAME_SCENE_HPP
#define IN_GAME_SCENE_HPP

#include <vector>
#include "Button.hpp"
#include "Scene.hpp"

class InGameScene : public Scene {
public:
    InGameScene(SceneManager* manager, IGameFacade* facade);

    void update() override;
    void draw() override;
    void onEnter() override;

private:
    std::vector<Button> actionButtons;
    Button closeOverlayButton;
    Button backToMenuButton;
    std::vector<Rectangle> tileRects;
    std::vector<Vector2> tokenPositions;
    std::vector<float> tokenPhases;
    float sceneTime;
    float overlayVisibility;

    void layoutUi(Rectangle screenRect, Rectangle& boardRect, Rectangle& sidebarRect);
    void updateAnimations(const Rectangle& boardRect);
    void updateBoardSelection();
    void drawBackground(Rectangle screenRect);
    void drawHeader(Rectangle screenRect);
    void drawBoard(const Rectangle& boardRect);
    void drawCenterPanel(const Rectangle& boardRect);
    void drawSidebar(const Rectangle& sidebarRect);
    void drawOverlay(Rectangle screenRect);
    Vector2 getTileCenter(const Rectangle& boardRect, int index) const;
    Rectangle getTileRect(const Rectangle& boardRect, int index) const;
};

#endif
