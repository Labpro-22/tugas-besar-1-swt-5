#include "../../include/views/MainMenuScene.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include "../../include/coredummy/IGameFacade.hpp"
#include "../../include/views/SceneManager.hpp"

namespace {
const Color kBgTop{10, 20, 44, 255};
const Color kBgBottom{23, 59, 103, 255};
const Color kBgGlow{244, 182, 94, 255};
const Color kPanel{16, 29, 61, 228};
const Color kPanelBorder{255, 255, 255, 42};
const Color kAccent{243, 120, 74, 255};
const Color kAccentAlt{69, 201, 173, 255};
const Color kText{245, 247, 255, 255};
const Color kSubtext{175, 190, 225, 255};
const Color kMuted{92, 109, 156, 255};
const Color kDanger{205, 74, 98, 255};

float easeTowards(float current, float target, float speed) {
    const float clamped = std::max(0.0f, std::min(speed, 1.0f));
    return current + (target - current) * clamped;
}
}

MainMenuScene::MainMenuScene(SceneManager* manager, IGameFacade* facade)
    : Scene(manager, facade),
      newGameButton("New Game", kAccent, kText),
      loadDemoButton("Load Demo", kAccentAlt, kText),
      quitButton("Quit", kDanger, kText),
      startGameButton("Start Demo", kAccent, kText),
      cancelButton("Cancel", Color{63, 79, 132, 255}, kText),
      plusButton("+", Color{55, 76, 145, 255}, kText),
      minusButton("-", Color{55, 76, 145, 255}, kText),
      showNewGameModal(false),
      playerCount(4),
      sceneTime(0.0f),
      modalVisibility(0.0f) {
    playerFields.emplace_back("Player 1");
    playerFields.emplace_back("Player 2");
    playerFields.emplace_back("Player 3");
    playerFields.emplace_back("Player 4");

    newGameButton.setOnClick([this]() { showNewGameModal = true; });
    loadDemoButton.setOnClick([this]() {
        gameFacade->loadDemoGame();
        sceneManager->setScene(SceneType::InGame);
    });
    quitButton.setOnClick([]() { CloseWindow(); });
    cancelButton.setOnClick([this]() { showNewGameModal = false; });
    plusButton.setOnClick([this]() { playerCount = std::min(4, playerCount + 1); });
    minusButton.setOnClick([this]() { playerCount = std::max(2, playerCount - 1); });
    startGameButton.setOnClick([this]() {
        std::vector<std::string> names;
        names.reserve(static_cast<std::size_t>(playerCount));
        for (int i = 0; i < playerCount; ++i) {
            names.push_back(playerFields[static_cast<std::size_t>(i)].getContent());
        }
        gameFacade->startNewGame(names);
        showNewGameModal = false;
        sceneManager->setScene(SceneType::InGame);
    });
}

void MainMenuScene::onEnter() {
    sceneTime = 0.0f;
    showNewGameModal = false;
    modalVisibility = 0.0f;
}

void MainMenuScene::layoutButtons(Rectangle screenRect) {
    const float width = std::min(210.0f, (screenRect.width - 180.0f) / 3.2f);
    const float y = screenRect.y + screenRect.height - 140.0f;
    const float left = screenRect.x + 72.0f;
    const float gap = 18.0f;

    newGameButton.setBoundary({left, y, width, 58.0f});
    loadDemoButton.setBoundary({left + width + gap, y, width, 58.0f});
    quitButton.setBoundary({left + (width + gap) * 2.0f, y, 150.0f, 58.0f});
}

void MainMenuScene::update() {
    const float dt = GetFrameTime();
    sceneTime += dt;
    modalVisibility = easeTowards(modalVisibility, showNewGameModal ? 1.0f : 0.0f, dt * 9.0f);

    Rectangle screenRect{0.0f, 0.0f, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
    layoutButtons(screenRect);

    if (IsKeyPressed(KEY_ESCAPE) && showNewGameModal) {
        showNewGameModal = false;
    }

    if (modalVisibility < 0.02f) {
        newGameButton.update();
        loadDemoButton.update();
        quitButton.update();
        return;
    }

    plusButton.update();
    minusButton.update();
    startGameButton.update();
    cancelButton.update();
    for (int i = 0; i < playerCount; ++i) {
        playerFields[static_cast<std::size_t>(i)].update();
    }
}

void MainMenuScene::drawAmbientDetails(Rectangle screenRect) {
    for (int i = 0; i < 7; ++i) {
        const float phase = sceneTime * (0.2f + 0.03f * static_cast<float>(i)) + static_cast<float>(i);
        const float radius = 130.0f + 70.0f * static_cast<float>(i);
        const int alpha = 7 + i * 2;
        DrawCircleLines(static_cast<int>(screenRect.width - 140.0f), 124, radius + std::sin(phase) * 8.0f, Fade(WHITE, alpha / 255.0f));
    }

    for (int i = 0; i < 18; ++i) {
        const float x = std::fmod(110.0f + static_cast<float>(i) * 91.0f + sceneTime * (18.0f + i), screenRect.width + 120.0f) - 60.0f;
        const float y = 60.0f + std::fmod(static_cast<float>(i) * 57.0f + sceneTime * (8.0f + i * 0.35f), screenRect.height - 120.0f);
        const float r = 2.5f + static_cast<float>(i % 3);
        DrawCircle(static_cast<int>(x), static_cast<int>(y), r, Fade(kText, 0.12f));
    }
}

void MainMenuScene::drawHero(Rectangle screenRect) {
    const float floatY = std::sin(sceneTime * 1.3f) * 8.0f;
    DrawText("NIMONSPOLI", 72, 74, 76, kText);
    DrawText("UI-layer preview with live motion, modal flow, and a mock game loop.", 76, 162, 27, kSubtext);
    DrawText("This is structured to match the scene-based design while staying independent from the unfinished core.", 76, 202, 22, kSubtext);

    Rectangle preview{screenRect.width - 520.0f, 84.0f + floatY, 430.0f, 304.0f};
    DrawRectangleRounded({preview.x + 8.0f, preview.y + 12.0f, preview.width, preview.height}, 0.08f, 12, Fade(BLACK, 0.2f));
    DrawRectangleRounded(preview, 0.08f, 12, Fade(kPanel, 0.95f));
    DrawRectangleRoundedLinesEx(preview, 0.08f, 12, 2.0f, kPanelBorder);

    Rectangle board{preview.x + 22.0f, preview.y + 22.0f, 234.0f, 234.0f};
    DrawRectangleRounded(board, 0.05f, 8, Color{229, 235, 217, 255});
    DrawRectangleLinesEx(board, 3.0f, Color{70, 87, 47, 255});
    for (int i = 0; i < 11; ++i) {
        DrawLineEx({board.x, board.y + i * 21.27f}, {board.x + board.width, board.y + i * 21.27f}, 1.0f, Fade(DARKGREEN, 0.24f));
        DrawLineEx({board.x + i * 21.27f, board.y}, {board.x + i * 21.27f, board.y + board.height}, 1.0f, Fade(DARKGREEN, 0.24f));
    }

    Rectangle ribbon{board.x + 46.0f, board.y + 75.0f, 142.0f, 76.0f};
    DrawRectanglePro({ribbon.x + ribbon.width * 0.5f, ribbon.y + ribbon.height * 0.5f, ribbon.width, ribbon.height}, {ribbon.width * 0.5f, ribbon.height * 0.5f}, -23.0f, Color{214, 77, 64, 255});
    DrawText("NIMONSPOLI", static_cast<int>(board.x + 55.0f), static_cast<int>(board.y + 103.0f), 22, WHITE);

    const float orbit = sceneTime * 1.2f;
    for (int i = 0; i < 4; ++i) {
        const float px = board.x + 42.0f + static_cast<float>(i) * 46.0f + std::cos(orbit + i) * 5.0f;
        const float py = board.y + 190.0f + std::sin(orbit * 1.4f + i * 0.8f) * 9.0f;
        const Color token = (i % 2 == 0) ? kAccent : kAccentAlt;
        DrawCircle(static_cast<int>(px), static_cast<int>(py), 10.0f, token);
        DrawCircleLines(static_cast<int>(px), static_cast<int>(py), 14.0f + std::sin(orbit + i) * 2.0f, Fade(token, 0.2f));
    }

    Rectangle side{preview.x + 274.0f, preview.y + 22.0f, 134.0f, 260.0f};
    DrawRectangleRounded(side, 0.12f, 10, Color{24, 43, 92, 255});
    DrawText("Preview", static_cast<int>(side.x + 16.0f), static_cast<int>(side.y + 16.0f), 24, kText);
    DrawText("- animated menu", static_cast<int>(side.x + 16.0f), static_cast<int>(side.y + 60.0f), 19, kSubtext);
    DrawText("- playable shell", static_cast<int>(side.x + 16.0f), static_cast<int>(side.y + 88.0f), 19, kSubtext);
    DrawText("- live board demo", static_cast<int>(side.x + 16.0f), static_cast<int>(side.y + 116.0f), 19, kSubtext);
    DrawText("- overlays & HUD", static_cast<int>(side.x + 16.0f), static_cast<int>(side.y + 144.0f), 19, kSubtext);
    DrawText("- mock facade", static_cast<int>(side.x + 16.0f), static_cast<int>(side.y + 172.0f), 19, kSubtext);

    Rectangle bar{side.x + 16.0f, side.y + 214.0f, side.width - 32.0f, 14.0f};
    DrawRectangleRounded(bar, 0.9f, 12, Color{44, 61, 118, 255});
    DrawRectangleRounded({bar.x, bar.y, bar.width * (0.45f + 0.35f * (0.5f + 0.5f * std::sin(sceneTime * 1.8f))), bar.height}, 0.9f, 12, kAccentAlt);
}

void MainMenuScene::drawFeatureCards(Rectangle screenRect) {
    struct FeatureCard {
        const char* title;
        const char* lines[3];
        Color accent;
    };

    const std::vector<FeatureCard> cards = {
        {"Board Shell", {"40 tiles", "selection", "animated tokens"}, Color{255, 141, 97, 255}},
        {"Scene Flow", {"main menu", "in-game HUD", "overlay transitions"}, Color{88, 213, 179, 255}},
        {"Facade Bridge", {"real backend", "can plug in", "without UI rewrite"}, Color{113, 160, 255, 255}}
    };

    const float startX = 72.0f;
    const float y = 420.0f;
    const float gap = 18.0f;
    const float cardW = (screenRect.width - startX * 2.0f - gap * 2.0f) / 3.0f;

    for (std::size_t i = 0; i < cards.size(); ++i) {
        const float bob = std::sin(sceneTime * 1.6f + static_cast<float>(i) * 0.9f) * 7.0f;
        Rectangle rect{startX + static_cast<float>(i) * (cardW + gap), y + bob, cardW, 166.0f};
        DrawRectangleRounded({rect.x + 4.0f, rect.y + 10.0f, rect.width, rect.height}, 0.14f, 10, Fade(BLACK, 0.18f));
        DrawRectangleRounded(rect, 0.14f, 10, Fade(kPanel, 0.93f));
        DrawRectangleRoundedLinesEx(rect, 0.14f, 10, 2.0f, kPanelBorder);
        DrawRectangleRounded({rect.x + 18.0f, rect.y + 18.0f, 72.0f, 12.0f}, 0.9f, 8, cards[i].accent);
        DrawText(cards[i].title, static_cast<int>(rect.x + 18.0f), static_cast<int>(rect.y + 46.0f), 28, kText);
        DrawText(cards[i].lines[0], static_cast<int>(rect.x + 18.0f), static_cast<int>(rect.y + 96.0f), 22, kSubtext);
        DrawText(cards[i].lines[1], static_cast<int>(rect.x + 18.0f), static_cast<int>(rect.y + 122.0f), 22, kSubtext);
        DrawText(cards[i].lines[2], static_cast<int>(rect.x + 18.0f), static_cast<int>(rect.y + 148.0f), 22, kSubtext);
    }
}

void MainMenuScene::drawNewGameModal(Rectangle screenRect) {
    if (modalVisibility <= 0.01f) {
        return;
    }

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.58f * modalVisibility));
    const float rise = (1.0f - modalVisibility) * 28.0f;
    Rectangle modal{screenRect.width * 0.5f - 300.0f, screenRect.height * 0.5f - 244.0f + rise, 600.0f, 488.0f};
    DrawRectangleRounded({modal.x + 8.0f, modal.y + 12.0f, modal.width, modal.height}, 0.08f, 12, Fade(BLACK, 0.22f * modalVisibility));
    DrawRectangleRounded(modal, 0.08f, 12, Fade(Color{17, 25, 58, 248}, modalVisibility));
    DrawRectangleRoundedLinesEx(modal, 0.08f, 12, 2.0f, Fade(WHITE, 0.18f * modalVisibility));

    DrawText("Setup New Game", static_cast<int>(modal.x + 28.0f), static_cast<int>(modal.y + 26.0f), 34, kText);
    DrawText("Use 2 to 4 players. Empty names will be filled automatically by the facade.", static_cast<int>(modal.x + 28.0f), static_cast<int>(modal.y + 72.0f), 20, kSubtext);

    DrawText("Players", static_cast<int>(modal.x + 28.0f), static_cast<int>(modal.y + 118.0f), 22, kText);
    minusButton.setBoundary({modal.x + 194.0f, modal.y + 104.0f, 52.0f, 46.0f});
    plusButton.setBoundary({modal.x + 318.0f, modal.y + 104.0f, 52.0f, 46.0f});
    DrawRectangleRounded({modal.x + 254.0f, modal.y + 104.0f, 56.0f, 46.0f}, 0.22f, 8, Color{34, 47, 93, 255});
    DrawText(std::to_string(playerCount).c_str(), static_cast<int>(modal.x + 277.0f), static_cast<int>(modal.y + 115.0f), 24, kText);
    plusButton.draw();
    minusButton.draw();

    for (int i = 0; i < 4; ++i) {
        const float fieldY = modal.y + 172.0f + static_cast<float>(i) * 62.0f;
        playerFields[static_cast<std::size_t>(i)].setBoundary({modal.x + 28.0f, fieldY, modal.width - 56.0f, 50.0f});
        playerFields[static_cast<std::size_t>(i)].draw();
        if (i >= playerCount) {
            DrawRectangleRounded({modal.x + 28.0f, fieldY, modal.width - 56.0f, 50.0f}, 0.22f, 8, Fade(BLACK, 0.45f));
            DrawText("unused", static_cast<int>(modal.x + modal.width - 124.0f), static_cast<int>(fieldY + 15.0f), 18, Fade(kSubtext, 0.9f));
        }
    }

    startGameButton.setBoundary({modal.x + modal.width - 230.0f, modal.y + modal.height - 78.0f, 160.0f, 52.0f});
    cancelButton.setBoundary({modal.x + modal.width - 402.0f, modal.y + modal.height - 78.0f, 146.0f, 52.0f});
    startGameButton.draw();
    cancelButton.draw();
}

void MainMenuScene::draw() {
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), kBgTop, kBgBottom);
    Rectangle screenRect{0.0f, 0.0f, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};

    DrawCircle(static_cast<int>(screenRect.width - 170.0f), 120, 150.0f, Fade(kBgGlow, 0.08f));
    DrawCircle(static_cast<int>(screenRect.width - 250.0f), 180, 220.0f, Fade(kAccent, 0.06f));

    drawAmbientDetails(screenRect);
    drawHero(screenRect);
    drawFeatureCards(screenRect);

    DrawText("Design target", 76, static_cast<int>(screenRect.height - 218.0f), 26, kText);
    DrawText("Menu starts fast, the in-game scene can run as a convincing standalone UI shell, and integration later only needs a facade adapter.", 76, static_cast<int>(screenRect.height - 182.0f), 22, kSubtext);
    DrawText("The mock facade keeps the board alive so this milestone can be reviewed before the real game logic is connected.", 76, static_cast<int>(screenRect.height - 150.0f), 22, kSubtext);

    newGameButton.draw();
    loadDemoButton.draw();
    quitButton.draw();
    drawNewGameModal(screenRect);
}
