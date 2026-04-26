#include "../../include/views/InGameScene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include "../../include/core/GameViewModel.hpp"
#include "../../include/core/IGameFacade.hpp"
#include "../../include/views/SceneManager.hpp"

namespace {
const Color kBgA{9, 18, 40, 255};
const Color kBgB{19, 56, 88, 255};
const Color kPanel{17, 28, 61, 232};
const Color kPanelBorder{255, 255, 255, 40};
const Color kBoardSurface{227, 235, 218, 255};
const Color kCenterSurface{205, 216, 198, 255};
const Color kText{243, 247, 255, 255};
const Color kSubtext{177, 190, 226, 255};
const Color kAccent{243, 120, 74, 255};
const Color kAccentAlt{73, 205, 170, 255};
const std::array<Color, 4> kPlayerColors = {
    Color{255, 113, 122, 255},
    Color{90, 194, 255, 255},
    Color{123, 230, 142, 255},
    Color{252, 192, 86, 255}
};

Color groupColor(const std::string& group) {
    if (group == "COKLAT") return Color{135, 86, 58, 255};
    if (group == "BIRU_MUDA") return Color{124, 215, 255, 255};
    if (group == "MERAH_MUDA") return Color{245, 120, 182, 255};
    if (group == "ORANGE") return Color{244, 154, 74, 255};
    if (group == "MERAH") return Color{228, 77, 75, 255};
    if (group == "KUNING") return Color{241, 213, 81, 255};
    if (group == "HIJAU") return Color{88, 191, 120, 255};
    if (group == "BIRU_TUA") return Color{65, 92, 202, 255};
    if (group == "ABU" || group == "ABU_ABU") return Color{149, 158, 176, 255};
    return Color{208, 211, 222, 255};
}

std::string kindLabel(TileKind kind) {
    switch (kind) {
        case TileKind::Street: return "Street";
        case TileKind::Railroad: return "Railroad";
        case TileKind::Utility: return "Utility";
        case TileKind::Card: return "Card";
        case TileKind::Festival: return "Festival";
        case TileKind::Tax: return "Tax";
        case TileKind::Special: return "Special";
    }
    return "Tile";
}

float ease(float current, float target, float rate) {
    const float clamped = std::max(0.0f, std::min(rate, 1.0f));
    return current + (target - current) * clamped;
}
}

InGameScene::InGameScene(SceneManager* manager, IGameFacade* facade)
    : Scene(manager, facade),
      closeOverlayButton("X", Color{194, 83, 102, 255}, kText),
      backToMenuButton("Menu", Color{74, 92, 151, 255}, kText),
      sceneTime(0.0f),
      overlayVisibility(0.0f) {
    const std::vector<std::pair<std::string, std::function<void()>>> specs = {
        {"Roll Dice", [this]() { gameFacade->rollDice(); }},
        {"Tile Detail", [this]() { gameFacade->openSelectedTileDetails(); }},
        {"Properties", [this]() { gameFacade->showCurrentPlayerProperties(); }},
        {"Cards", [this]() { gameFacade->showCardPopup(); }},
        {"Festival", [this]() { gameFacade->showFestivalPopup(); }},
        {"Tax", [this]() { gameFacade->showTaxPopup(); }},
        {"Auction", [this]() { gameFacade->showAuctionPopup(); }},
        {"Save / Load", [this]() { gameFacade->showSavePopup(); }},
        {"End Turn", [this]() { gameFacade->advanceTurn(); }},
        {"Victory", [this]() { gameFacade->showVictoryPopup(); }}
    };

    for (const auto& spec : specs) {
        Button button(spec.first, kAccent, kText);
        button.setOnClick(spec.second);
        actionButtons.push_back(button);
    }

    closeOverlayButton.setOnClick([this]() { gameFacade->closeOverlay(); });
    backToMenuButton.setOnClick([this]() { sceneManager->setScene(SceneType::MainMenu); });
}

void InGameScene::onEnter() {
    sceneTime = 0.0f;
    overlayVisibility = 0.0f;
    tokenPositions.clear();
    tokenPhases.clear();
}

void InGameScene::layoutUi(Rectangle screenRect, Rectangle& boardRect, Rectangle& sidebarRect) {
    const float sideMargin = 22.0f;
    const float topMargin = 96.0f;
    const float gap = 18.0f;
    const float sidebarWidth = std::max(340.0f, std::min(screenRect.width * 0.29f, 390.0f));
    const float boardSize = std::min(screenRect.height - topMargin - 28.0f, screenRect.width - sidebarWidth - gap - sideMargin * 2.0f);

    boardRect = {sideMargin, topMargin, boardSize, boardSize};
    sidebarRect = {boardRect.x + boardRect.width + gap, topMargin, sidebarWidth, boardSize};
}

Rectangle InGameScene::getTileRect(const Rectangle& boardRect, int index) const {
    const float cell = boardRect.width / 11.0f;

    if (index >= 0 && index <= 10) {
        return {boardRect.x + boardRect.width - (static_cast<float>(index) + 1.0f) * cell, boardRect.y + boardRect.height - cell, cell, cell};
    }
    if (index >= 11 && index <= 19) {
        return {boardRect.x, boardRect.y + boardRect.height - (static_cast<float>(index - 10) + 1.0f) * cell, cell, cell};
    }
    if (index >= 20 && index <= 30) {
        return {boardRect.x + static_cast<float>(index - 20) * cell, boardRect.y, cell, cell};
    }
    return {boardRect.x + boardRect.width - cell, boardRect.y + static_cast<float>(index - 30) * cell, cell, cell};
}

Vector2 InGameScene::getTileCenter(const Rectangle& boardRect, int index) const {
    const Rectangle rect = getTileRect(boardRect, index);
    return {rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f};
}

void InGameScene::updateAnimations(const Rectangle& boardRect) {
    const GameViewModel& vm = gameFacade->getViewModel();
    const std::size_t playerCount = vm.players.size();

    if (tokenPositions.size() != playerCount) {
        tokenPositions.assign(playerCount, Vector2{0.0f, 0.0f});
        tokenPhases.assign(playerCount, 0.0f);
        for (std::size_t i = 0; i < playerCount; ++i) {
            tokenPositions[i] = getTileCenter(boardRect, vm.players[i].position);
            tokenPhases[i] = static_cast<float>(i) * 1.37f;
        }
    }

    const float dt = GetFrameTime();
    for (std::size_t i = 0; i < playerCount; ++i) {
        const Vector2 target = getTileCenter(boardRect, vm.players[i].position);
        tokenPositions[i].x = ease(tokenPositions[i].x, target.x, dt * 6.5f);
        tokenPositions[i].y = ease(tokenPositions[i].y, target.y, dt * 6.5f);
        tokenPhases[i] += dt * (1.2f + static_cast<float>(i) * 0.15f);
    }

    overlayVisibility = ease(
        overlayVisibility,
        vm.overlay.type == OverlayType::None ? 0.0f : 1.0f,
        dt * 8.0f
    );
}

void InGameScene::updateBoardSelection() {
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return;
    }

    const Vector2 mouse = GetMousePosition();
    for (std::size_t i = 0; i < tileRects.size(); ++i) {
        if (CheckCollisionPointRec(mouse, tileRects[i])) {
            gameFacade->selectTile(static_cast<int>(i));
            break;
        }
    }
}

void InGameScene::update() {
    sceneTime += GetFrameTime();

    Rectangle screenRect{0.0f, 0.0f, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
    Rectangle boardRect{};
    Rectangle sidebarRect{};
    layoutUi(screenRect, boardRect, sidebarRect);

    tileRects.clear();
    tileRects.reserve(40);
    for (int i = 0; i < 40; ++i) {
        tileRects.push_back(getTileRect(boardRect, i));
    }

    updateAnimations(boardRect);

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (gameFacade->getViewModel().overlay.type != OverlayType::None) {
            gameFacade->closeOverlay();
        } else {
            sceneManager->setScene(SceneType::MainMenu);
            return;
        }
    }

    updateBoardSelection();

    backToMenuButton.setBoundary({screenRect.width - 136.0f, 24.0f, 112.0f, 44.0f});
    backToMenuButton.update();

    const float btnW = (sidebarRect.width - 18.0f) * 0.5f;
    const float btnH = 46.0f;
    const float startY = sidebarRect.y + 244.0f;
    for (std::size_t i = 0; i < actionButtons.size(); ++i) {
        const int row = static_cast<int>(i) / 2;
        const int col = static_cast<int>(i) % 2;
        actionButtons[i].setBoundary({sidebarRect.x + static_cast<float>(col) * (btnW + 18.0f), startY + static_cast<float>(row) * 56.0f, btnW, btnH});
        actionButtons[i].update();
    }

    if (overlayVisibility > 0.01f) {
        closeOverlayButton.setBoundary({screenRect.width * 0.5f + 274.0f, screenRect.height * 0.5f - 244.0f, 52.0f, 40.0f});
        closeOverlayButton.update();
    }
}

void InGameScene::drawBackground(Rectangle screenRect) {
    DrawRectangleGradientV(0, 0, static_cast<int>(screenRect.width), static_cast<int>(screenRect.height), kBgA, kBgB);

    for (int i = 0; i < 8; ++i) {
        const float phase = sceneTime * (0.18f + static_cast<float>(i) * 0.02f);
        const float radius = 140.0f + static_cast<float>(i) * 70.0f + std::sin(phase + i) * 8.0f;
        DrawCircleLines(static_cast<int>(screenRect.width - 140.0f), 126, radius, Fade(WHITE, 0.035f));
    }

    for (int i = 0; i < 24; ++i) {
        const float x = std::fmod(70.0f + static_cast<float>(i) * 77.0f + sceneTime * (12.0f + i), screenRect.width + 80.0f) - 40.0f;
        const float y = 82.0f + std::fmod(static_cast<float>(i) * 49.0f + sceneTime * (9.0f + i * 0.2f), screenRect.height - 140.0f);
        DrawCircle(static_cast<int>(x), static_cast<int>(y), 2.0f + static_cast<float>(i % 3), Fade(kText, 0.08f));
    }
}

void InGameScene::drawHeader(Rectangle) {
    const GameViewModel& vm = gameFacade->getViewModel();
    DrawText("Nimonspoli", 24, 20, 36, kText);
    DrawText(vm.statusLine.c_str(), 24, 60, 21, kSubtext);

    Rectangle badge{420.0f, 22.0f, 228.0f, 48.0f};
    DrawRectangleRounded(badge, 0.25f, 8, Color{24, 38, 82, 232});
    DrawRectangleRoundedLinesEx(badge, 0.25f, 8, 1.5f, kPanelBorder);
    std::string turnText = "TURN " + std::to_string(vm.currentTurn) + " / " + std::to_string(vm.maxTurn);
    DrawText(turnText.c_str(), static_cast<int>(badge.x + 24.0f), static_cast<int>(badge.y + 13.0f), 22, kText);

    backToMenuButton.draw();
}

void InGameScene::drawCenterPanel(const Rectangle& boardRect) {
    const float cell = boardRect.width / 11.0f;
    Rectangle center{boardRect.x + cell, boardRect.y + cell, boardRect.width - 2.0f * cell, boardRect.height - 2.0f * cell};
    DrawRectangleRounded(center, 0.05f, 10, kCenterSurface);
    DrawRectangleRoundedLinesEx(center, 0.05f, 10, 3.0f, Color{54, 71, 46, 255});

    Rectangle ribbon{center.x + center.width * 0.26f, center.y + center.height * 0.44f, center.width * 0.5f, 74.0f};
    DrawRectanglePro({ribbon.x + ribbon.width * 0.5f, ribbon.y + ribbon.height * 0.5f, ribbon.width, ribbon.height}, {ribbon.width * 0.5f, ribbon.height * 0.5f}, -24.0f, Color{219, 76, 63, 255});
    DrawText("NIMONSPOLI", static_cast<int>(center.x + center.width * 0.28f), static_cast<int>(center.y + center.height * 0.45f), 40, WHITE);

    const float cardOffset = std::sin(sceneTime * 1.4f) * 7.0f;
    Rectangle cardA{center.x + 74.0f, center.y + 84.0f + cardOffset, 110.0f, 148.0f};
    Rectangle cardB{center.x + center.width - 184.0f, center.y + center.height - 226.0f - cardOffset, 110.0f, 148.0f};
    DrawRectangleRounded(cardA, 0.12f, 8, Color{88, 183, 255, 255});
    DrawRectangleRounded(cardB, 0.12f, 8, Color{255, 179, 86, 255});
    DrawRectangleRoundedLinesEx(cardA, 0.12f, 8, 2.0f, Fade(BLACK, 0.18f));
    DrawRectangleRoundedLinesEx(cardB, 0.12f, 8, 2.0f, Fade(BLACK, 0.18f));
    DrawText("SKILL", static_cast<int>(cardA.x + 18.0f), static_cast<int>(cardA.y + 58.0f), 28, WHITE);
    DrawText("CHANCE", static_cast<int>(cardB.x + 6.0f), static_cast<int>(cardB.y + 58.0f), 24, WHITE);

    DrawCircle(static_cast<int>(center.x + center.width * 0.5f), static_cast<int>(center.y + center.height * 0.48f), 132.0f + std::sin(sceneTime * 1.8f) * 10.0f, Fade(kAccentAlt, 0.06f));
    DrawCircleLines(static_cast<int>(center.x + center.width * 0.5f), static_cast<int>(center.y + center.height * 0.48f), 150.0f + std::sin(sceneTime * 1.2f) * 9.0f, Fade(kAccent, 0.14f));
}

void InGameScene::drawBoard(const Rectangle& boardRect) {
    const GameViewModel& vm = gameFacade->getViewModel();

    DrawRectangleRounded({boardRect.x + 6.0f, boardRect.y + 8.0f, boardRect.width, boardRect.height}, 0.04f, 10, Fade(BLACK, 0.2f));
    DrawRectangleRounded(boardRect, 0.04f, 10, kBoardSurface);
    DrawRectangleRoundedLinesEx(boardRect, 0.04f, 10, 3.0f, Color{61, 79, 44, 255});

    for (std::size_t i = 0; i < vm.board.size() && i < tileRects.size(); ++i) {
        const TileViewData& tile = vm.board[i];
        Rectangle rect = tileRects[i];
        const bool selected = static_cast<int>(i) == vm.selectedTileIndex;

        DrawRectangleRec(rect, kBoardSurface);
        DrawRectangleLinesEx(rect, 1.0f, Fade(DARKGRAY, 0.35f));

        Rectangle band = rect;
        if (i <= 10) {
            band.height = 18.0f;
            band.y = rect.y + rect.height - band.height;
        } else if (i <= 19) {
            band.width = 18.0f;
        } else if (i <= 30) {
            band.height = 18.0f;
        } else {
            band.width = 18.0f;
            band.x = rect.x + rect.width - band.width;
        }
        DrawRectangleRec(band, groupColor(tile.colorGroup));

        if (selected) {
            const float pulse = 0.5f + 0.5f * std::sin(sceneTime * 3.0f);
            DrawRectangleLinesEx(rect, 3.0f + pulse * 2.0f, kAccent);
        }

        DrawText(tile.code.c_str(), static_cast<int>(rect.x + 6.0f), static_cast<int>(rect.y + 6.0f), 18, Color{40, 50, 58, 255});
        DrawText(tile.name.c_str(), static_cast<int>(rect.x + 6.0f), static_cast<int>(rect.y + 28.0f), 11, Color{54, 58, 62, 255});

        if (tile.ownerIndex >= 0) {
            DrawCircle(static_cast<int>(rect.x + rect.width - 16.0f), static_cast<int>(rect.y + 16.0f), 8.0f, kPlayerColors[static_cast<std::size_t>(tile.ownerIndex % 4)]);
        }
        if (tile.status == PropertyStatusView::Mortgaged) {
            DrawText("M", static_cast<int>(rect.x + rect.width - 28.0f), static_cast<int>(rect.y + 28.0f), 16, Color{155, 44, 82, 255});
        }

        for (int h = 0; h < tile.houses && h < 4; ++h) {
            DrawRectangle(static_cast<int>(rect.x + 8.0f + h * 12.0f), static_cast<int>(rect.y + rect.height - 36.0f), 8, 8, Color{52, 184, 91, 255});
        }
        if (tile.hotel) {
            DrawRectangle(static_cast<int>(rect.x + 8.0f), static_cast<int>(rect.y + rect.height - 36.0f), 18, 10, Color{210, 52, 67, 255});
        }
        if (tile.festivalMultiplier > 1) {
            std::string fest = "x" + std::to_string(tile.festivalMultiplier);
            DrawText(fest.c_str(), static_cast<int>(rect.x + rect.width - 28.0f), static_cast<int>(rect.y + rect.height - 24.0f), 14, Color{140, 72, 208, 255});
        }
    }

    drawCenterPanel(boardRect);

    for (std::size_t i = 0; i < tokenPositions.size(); ++i) {
        const float bob = std::sin(tokenPhases[i] * 2.0f) * 4.0f;
        const float ring = 15.0f + std::sin(tokenPhases[i] * 2.8f) * 2.0f;
        const Vector2 pos{tokenPositions[i].x, tokenPositions[i].y + bob};
        const Color tint = kPlayerColors[i % 4];
        DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), ring, Fade(tint, 0.24f));
        DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 11.0f, tint);
        DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), 11.0f, Fade(WHITE, 0.55f));
        DrawText(std::to_string(static_cast<int>(i) + 1).c_str(), static_cast<int>(pos.x - 4.0f), static_cast<int>(pos.y - 6.0f), 13, WHITE);
    }
}

void InGameScene::drawSidebar(const Rectangle& sidebarRect) {
    const GameViewModel& vm = gameFacade->getViewModel();
    if (vm.players.empty() || vm.board.empty()) {
        return;
    }

    DrawRectangleRounded({sidebarRect.x + 5.0f, sidebarRect.y + 8.0f, sidebarRect.width, sidebarRect.height}, 0.04f, 10, Fade(BLACK, 0.2f));
    DrawRectangleRounded(sidebarRect, 0.04f, 10, kPanel);
    DrawRectangleRoundedLinesEx(sidebarRect, 0.04f, 10, 2.0f, kPanelBorder);

    const PlayerViewData& active = vm.players[vm.currentPlayerIndex];
    DrawText("Active Player", static_cast<int>(sidebarRect.x + 18.0f), static_cast<int>(sidebarRect.y + 18.0f), 26, kText);
    DrawCircle(static_cast<int>(sidebarRect.x + 32.0f), static_cast<int>(sidebarRect.y + 70.0f), 12.0f, kPlayerColors[static_cast<std::size_t>(vm.currentPlayerIndex % 4)]);
    DrawText(active.name.c_str(), static_cast<int>(sidebarRect.x + 54.0f), static_cast<int>(sidebarRect.y + 56.0f), 28, kText);
    DrawText(("Money: M" + std::to_string(active.money)).c_str(), static_cast<int>(sidebarRect.x + 18.0f), static_cast<int>(sidebarRect.y + 94.0f), 22, kSubtext);
    DrawText(("Position: " + vm.board[active.position].code + " - " + vm.board[active.position].name).c_str(), static_cast<int>(sidebarRect.x + 18.0f), static_cast<int>(sidebarRect.y + 124.0f), 20, kSubtext);
    DrawText(vm.footerHint.c_str(), static_cast<int>(sidebarRect.x + 18.0f), static_cast<int>(sidebarRect.y + 156.0f), 18, Fade(kSubtext, 0.92f));

    Rectangle wave{sidebarRect.x + 18.0f, sidebarRect.y + 194.0f, sidebarRect.width - 36.0f, 18.0f};
    DrawRectangleRounded(wave, 0.8f, 10, Color{33, 46, 94, 255});
    const float progress = 0.45f + 0.35f * (0.5f + 0.5f * std::sin(sceneTime * 1.8f));
    DrawRectangleRounded({wave.x, wave.y, wave.width * progress, wave.height}, 0.8f, 10, kAccentAlt);

    for (Button& button : actionButtons) {
        button.draw();
    }

    const TileViewData& selected = vm.board[vm.selectedTileIndex];
    Rectangle inspect{sidebarRect.x, sidebarRect.y + 418.0f, sidebarRect.width, 112.0f};
    DrawText("Selected Tile", static_cast<int>(inspect.x + 18.0f), static_cast<int>(inspect.y + 2.0f), 24, kText);
    DrawText((selected.code + " - " + selected.name).c_str(), static_cast<int>(inspect.x + 18.0f), static_cast<int>(inspect.y + 34.0f), 22, kText);
    DrawText(kindLabel(selected.kind).c_str(), static_cast<int>(inspect.x + 18.0f), static_cast<int>(inspect.y + 64.0f), 20, kSubtext);
    if (selected.kind == TileKind::Street || selected.kind == TileKind::Railroad || selected.kind == TileKind::Utility) {
        DrawText(("Price: M" + std::to_string(selected.price)).c_str(), static_cast<int>(inspect.x + 138.0f), static_cast<int>(inspect.y + 64.0f), 20, kSubtext);
    }

    Rectangle playersPanel{sidebarRect.x, sidebarRect.y + 546.0f, sidebarRect.width, 158.0f};
    DrawText("All Players", static_cast<int>(playersPanel.x + 18.0f), static_cast<int>(playersPanel.y + 2.0f), 24, kText);
    for (std::size_t i = 0; i < vm.players.size(); ++i) {
        const float y = playersPanel.y + 34.0f + static_cast<float>(i) * 28.0f;
        DrawCircle(static_cast<int>(playersPanel.x + 22.0f), static_cast<int>(y + 8.0f), 7.0f, kPlayerColors[i % 4]);
        std::string row = vm.players[i].name + " | M" + std::to_string(vm.players[i].money) + " | " + vm.players[i].status;
        DrawText(row.c_str(), static_cast<int>(playersPanel.x + 38.0f), static_cast<int>(y), 19, vm.players[i].isCurrent ? kText : kSubtext);
    }

    Rectangle cardsPanel{sidebarRect.x, sidebarRect.y + 704.0f, sidebarRect.width, 86.0f};
    DrawText("Hand Cards", static_cast<int>(cardsPanel.x + 18.0f), static_cast<int>(cardsPanel.y + 2.0f), 24, kText);
    for (std::size_t i = 0; i < vm.hand.size(); ++i) {
        const float sway = std::sin(sceneTime * 1.8f + static_cast<float>(i)) * 4.0f;
        Rectangle card{cardsPanel.x + 18.0f + static_cast<float>(i) * 108.0f, cardsPanel.y + 34.0f + sway, 96.0f, 40.0f};
        DrawRectangleRounded(card, 0.2f, 8, Color{31, 48, 104, 255});
        DrawRectangleRoundedLinesEx(card, 0.2f, 8, 1.5f, Fade(WHITE, 0.18f));
        DrawText(vm.hand[i].title.c_str(), static_cast<int>(card.x + 8.0f), static_cast<int>(card.y + 11.0f), 15, kText);
    }

    Rectangle logsPanel{sidebarRect.x, sidebarRect.y + 796.0f, sidebarRect.width, sidebarRect.height - 796.0f};
    DrawText("Recent Logs", static_cast<int>(logsPanel.x + 18.0f), static_cast<int>(logsPanel.y + 2.0f), 24, kText);
    const int logCount = std::min<int>(3, vm.logs.size());
    for (int i = 0; i < logCount; ++i) {
        const LogEntryViewData& log = vm.logs[static_cast<std::size_t>(i)];
        const float y = logsPanel.y + 32.0f + static_cast<float>(i) * 42.0f;
        DrawText(("[T" + std::to_string(log.turn) + "] " + log.actor + " | " + log.type).c_str(), static_cast<int>(logsPanel.x + 18.0f), static_cast<int>(y), 17, kText);
        DrawText(log.detail.c_str(), static_cast<int>(logsPanel.x + 18.0f), static_cast<int>(y + 18.0f), 16, kSubtext);
    }
}

void InGameScene::drawOverlay(Rectangle screenRect) {
    const GameViewModel& vm = gameFacade->getViewModel();
    if (overlayVisibility <= 0.01f || vm.overlay.type == OverlayType::None) {
        return;
    }

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f * overlayVisibility));
    const float rise = (1.0f - overlayVisibility) * 26.0f;
    Rectangle panel{screenRect.width * 0.5f - 320.0f, screenRect.height * 0.5f - 260.0f + rise, 640.0f, 520.0f};
    DrawRectangleRounded({panel.x + 6.0f, panel.y + 10.0f, panel.width, panel.height}, 0.08f, 10, Fade(BLACK, 0.26f * overlayVisibility));
    DrawRectangleRounded(panel, 0.08f, 10, Fade(Color{15, 23, 53, 245}, overlayVisibility));
    DrawRectangleRoundedLinesEx(panel, 0.08f, 10, 2.0f, Fade(WHITE, 0.18f * overlayVisibility));

    DrawText(vm.overlay.title.c_str(), static_cast<int>(panel.x + 24.0f), static_cast<int>(panel.y + 22.0f), 34, kText);
    closeOverlayButton.draw();

    float y = panel.y + 86.0f;
    for (const std::string& line : vm.overlay.lines) {
        DrawText(line.c_str(), static_cast<int>(panel.x + 26.0f), static_cast<int>(y), 22, kSubtext);
        y += 34.0f;
        if (y > panel.y + panel.height - 100.0f) {
            break;
        }
    }

    DrawRectangleRounded({panel.x + 24.0f, panel.y + panel.height - 76.0f, panel.width - 48.0f, 48.0f}, 0.2f, 8, Color{27, 40, 87, 255});
    DrawText(vm.overlay.footer.c_str(), static_cast<int>(panel.x + 38.0f), static_cast<int>(panel.y + panel.height - 62.0f), 20, kText);
}

void InGameScene::draw() {
    Rectangle screenRect{0.0f, 0.0f, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
    Rectangle boardRect{};
    Rectangle sidebarRect{};
    layoutUi(screenRect, boardRect, sidebarRect);

    drawBackground(screenRect);
    drawHeader(screenRect);
    drawBoard(boardRect);
    drawSidebar(sidebarRect);
    drawOverlay(screenRect);
}
