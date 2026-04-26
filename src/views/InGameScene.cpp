#include "../../include/views/InGameScene.hpp"
#include "../../include/coredummy/GameViewModel.hpp"
#include "../../include/coredummy/IGameFacade.hpp"
#include "../../include/views/SceneManager.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace {
const Color kBgA{230,245,210,255};
const Color kBgB{255,248,195,255};
const Color kPanel{245,255,230,230};
const Color kPanelBorder{160,200,100,180};
const Color kBoardSurf{240,248,220,255};
const Color kCenterSurf{225,242,200,255};
const Color kText{45,75,15,255};
const Color kSubtext{90,130,45,255};
const Color kAccent{255,190,30,255};
const Color kAccentAlt{80,175,50,255};
const Color kDanger{210,70,50,255};
const std::array<Color,4> kTokens = {
    Color{255,190,30,255},
    Color{80,175,50,255},
    Color{255,120,60,255},
    Color{60,160,220,255},
};

Color groupColor(const std::string& group) {
    if (group == "COKLAT") return {135,86,58,255};
    if (group == "BIRU_MUDA") return {124,215,255,255};
    if (group == "MERAH_MUDA") return {245,120,182,255};
    if (group == "ORANGE") return {244,154,74,255};
    if (group == "MERAH") return {228,77,75,255};
    if (group == "KUNING") return {241,213,81,255};
    if (group == "HIJAU") return {88,191,120,255};
    if (group == "BIRU_TUA") return {65,92,202,255};
    if (group == "ABU" || group == "ABU_ABU") return {149,158,176,255};
    return {200,210,185,255};
}

std::string statusText(PropertyStatusView status) {
    switch (status) {
        case PropertyStatusView::Bank: return "BANK";
        case PropertyStatusView::Owned: return "OWNED";
        case PropertyStatusView::Mortgaged: return "GADAI";
    }
    return "";
}

float ease(float current, float target, float speed) {
    float clamped = std::max(0.0f, std::min(speed, 1.0f));
    return current + (target - current) * clamped;
}

void drawSmallFlower(float cx, float cy, float r, float angle, float alpha) {
    for (int i = 0; i < 5; ++i) {
        float a = angle + i * (6.28318f / 5.0f);
        DrawCircle(static_cast<int>(cx + r * 1.3f * std::cos(a)),
                   static_cast<int>(cy + r * 1.3f * std::sin(a)),
                   r, Fade(kAccent, alpha));
    }
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy), r * 0.6f, Fade({255,160,30,255}, alpha));
}
}

InGameScene::InGameScene(SceneManager* manager, IGameFacade* facade)
    : Scene(manager, facade),
      closeOverlayButton("X", kDanger, {255,255,255,255}),
      backToMenuButton("Menu", kSubtext, {255,255,255,255}),
      sceneTime(0.0f),
      overlayVisibility(0.0f) {

    const std::vector<std::pair<std::string, std::function<void()>>> specs = {
        {"Lempar Dadu", [this]() { gameFacade->rollDice(); }},
        {"Beli", [this]() { gameFacade->buyCurrentProperty(); }},
        {"Info Petak", [this]() { gameFacade->openSelectedTileDetails(); }},
        {"Properti", [this]() { gameFacade->showCurrentPlayerProperties(); }},
        {"Kartu", [this]() { gameFacade->showCardPopup(); }},
        {"Festival", [this]() { gameFacade->showFestivalPopup(); }},
        {"Pajak", [this]() { gameFacade->showTaxPopup(); }},
        {"Lelang", [this]() { gameFacade->showAuctionPopup(); }},
        {"Simpan", [this]() { gameFacade->showSavePopup(); }},
        {"Akhir Giliran", [this]() { gameFacade->advanceTurn(); }},
        {"Kemenangan", [this]() { gameFacade->showVictoryPopup(); }},
    };

    for (const auto& spec : specs) {
        Button button(spec.first, kAccentAlt, {255,255,255,255});
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
    const float sideMargin = 20.0f;
    const float topMargin = 88.0f;
    const float gap = 16.0f;
    const float sidebarW = std::max(330.0f, std::min(screenRect.width * 0.28f, 380.0f));
    const float boardSize = std::min(screenRect.height - topMargin - 24.0f,
                                     screenRect.width - sidebarW - gap - sideMargin * 2.0f);
    boardRect = {sideMargin, topMargin, boardSize, boardSize};
    sidebarRect = {boardRect.x + boardRect.width + gap, topMargin, sidebarW, boardSize};
}

Rectangle InGameScene::getTileRect(const Rectangle& boardRect, int index) const {
    const float cell = boardRect.width / 11.0f;
    if (index >= 0 && index <= 10) {
        return {boardRect.x + boardRect.width - (index + 1.0f) * cell,
                boardRect.y + boardRect.height - cell, cell, cell};
    }
    if (index >= 11 && index <= 19) {
        return {boardRect.x, boardRect.y + boardRect.height - (index - 10 + 1.0f) * cell, cell, cell};
    }
    if (index >= 20 && index <= 30) {
        return {boardRect.x + (index - 20.0f) * cell, boardRect.y, cell, cell};
    }
    return {boardRect.x + boardRect.width - cell, boardRect.y + (index - 30.0f) * cell, cell, cell};
}

Vector2 InGameScene::getTileCenter(const Rectangle& boardRect, int index) const {
    Rectangle r = getTileRect(boardRect, index);
    return {r.x + r.width * 0.5f, r.y + r.height * 0.5f};
}

void InGameScene::updateAnimations(const Rectangle& boardRect) {
    const GameViewModel& vm = gameFacade->getViewModel();
    const std::size_t playerCount = vm.players.size();
    if (tokenPositions.size() != playerCount) {
        tokenPositions.assign(playerCount, {0,0});
        tokenPhases.assign(playerCount, 0.0f);
        for (std::size_t i = 0; i < playerCount; ++i) {
            tokenPositions[i] = getTileCenter(boardRect, vm.players[i].position);
            tokenPhases[i] = static_cast<float>(i) * 1.37f;
        }
    }

    float dt = GetFrameTime();
    for (std::size_t i = 0; i < playerCount; ++i) {
        Vector2 target = getTileCenter(boardRect, vm.players[i].position);
        tokenPositions[i].x = ease(tokenPositions[i].x, target.x, dt * 6.0f);
        tokenPositions[i].y = ease(tokenPositions[i].y, target.y, dt * 6.0f);
        tokenPhases[i] += dt * (1.1f + i * 0.12f);
    }

    overlayVisibility = ease(overlayVisibility,
                             vm.overlay.type == OverlayType::None ? 0.0f : 1.0f,
                             dt * 8.0f);
}

void InGameScene::updateBoardSelection() {
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;
    Vector2 mouse = GetMousePosition();
    for (std::size_t i = 0; i < tileRects.size(); ++i) {
        if (CheckCollisionPointRec(mouse, tileRects[i])) {
            gameFacade->selectTile(static_cast<int>(i));
            return;
        }
    }
}

void InGameScene::update() {
    sceneTime += GetFrameTime();
    Rectangle screen{0,0,static_cast<float>(GetScreenWidth()),static_cast<float>(GetScreenHeight())};
    Rectangle boardRect{}, sidebarRect{};
    layoutUi(screen, boardRect, sidebarRect);

    const GameViewModel& vm = gameFacade->getViewModel();
    tileRects.clear();
    for (int i = 0; i < static_cast<int>(vm.board.size()); ++i) {
        tileRects.push_back(getTileRect(boardRect, i));
    }

    updateAnimations(boardRect);
    updateBoardSelection();

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (vm.overlay.type != OverlayType::None) gameFacade->closeOverlay();
        else sceneManager->setScene(SceneType::MainMenu);
    }

    backToMenuButton.setBoundary({screen.width - 130.0f, 22.0f, 106.0f, 42.0f});
    backToMenuButton.update();

    float buttonW = (sidebarRect.width - 14.0f) * 0.5f;
    float buttonH = 44.0f;
    float startY = sidebarRect.y + 238.0f;
    for (std::size_t i = 0; i < actionButtons.size(); ++i) {
        int row = static_cast<int>(i) / 2;
        int col = static_cast<int>(i) % 2;
        actionButtons[i].setBoundary({sidebarRect.x + col * (buttonW + 14.0f),
                                      startY + row * 54.0f,
                                      buttonW, buttonH});
        actionButtons[i].update();
    }

    if (overlayVisibility > 0.01f) {
        closeOverlayButton.setBoundary({screen.width * 0.5f + 268.0f, screen.height * 0.5f - 240.0f, 50.0f, 38.0f});
        closeOverlayButton.update();
    }
}

void InGameScene::drawBackground(Rectangle screenRect) {
    DrawRectangleGradientV(0,0,static_cast<int>(screenRect.width),static_cast<int>(screenRect.height),kBgA,kBgB);
    drawSmallFlower(40,40,18,sceneTime * 0.3f,0.3f);
    drawSmallFlower(screenRect.width - 50,50,14,sceneTime * 0.35f + 1,0.25f);
    drawSmallFlower(30,screenRect.height - 50,12,sceneTime * 0.4f + 2,0.25f);
    drawSmallFlower(screenRect.width - 40,screenRect.height - 40,16,sceneTime * 0.3f + 0.5f,0.25f);
}

void InGameScene::drawHeader(Rectangle) {
    const GameViewModel& vm = gameFacade->getViewModel();
    DrawText("Nimonspoli",22,16,34,kText);
    DrawText(vm.statusLine.c_str(),22,52,20,kSubtext);
    Rectangle badge{380,18,240,46};
    DrawRectangleRounded(badge,0.28f,8,Fade(kAccent,0.2f));
    DrawRectangleRoundedLinesEx(badge,0.28f,8,1.5f,Fade(kPanelBorder,0.8f));
    std::string turnText = "Turn " + std::to_string(vm.currentTurn) + " / " + std::to_string(vm.maxTurn);
    DrawText(turnText.c_str(),static_cast<int>(badge.x + 20),static_cast<int>(badge.y + 12),22,kText);
    backToMenuButton.draw();
}

void InGameScene::drawCenterPanel(const Rectangle& boardRect) {
    float cell = boardRect.width / 11.0f;
    Rectangle center{boardRect.x + cell, boardRect.y + cell, boardRect.width - 2.0f * cell, boardRect.height - 2.0f * cell};
    DrawRectangleRounded(center,0.05f,10,kCenterSurf);
    DrawRectangleRoundedLinesEx(center,0.05f,10,3,Fade(kAccentAlt,0.6f));
    DrawText("NIMONSPOLI",static_cast<int>(center.x + center.width * 0.27f),static_cast<int>(center.y + center.height * 0.43f),38,kText);
}

void InGameScene::drawBoard(const Rectangle& boardRect) {
    const GameViewModel& vm = gameFacade->getViewModel();
    DrawRectangleRounded({boardRect.x + 5, boardRect.y + 7, boardRect.width, boardRect.height},0.04f,10,Fade(kText,0.08f));
    DrawRectangleRounded(boardRect,0.04f,10,kBoardSurf);
    DrawRectangleRoundedLinesEx(boardRect,0.04f,10,3,Fade(kAccentAlt,0.7f));

    if (vm.board.empty()) {
        drawCenterPanel(boardRect);
        DrawText("Board belum tersedia", static_cast<int>(boardRect.x + 40), static_cast<int>(boardRect.y + 40), 24, kDanger);
        return;
    }

    for (std::size_t i = 0; i < vm.board.size() && i < tileRects.size(); ++i) {
        const TileViewData& tile = vm.board[i];
        Rectangle r = tileRects[i];
        bool selected = static_cast<int>(i) == vm.selectedTileIndex;
        DrawRectangleRec(r,kBoardSurf);
        DrawRectangleLinesEx(r,1,Fade(kPanelBorder,0.5f));

        Rectangle band = r;
        if (i <= 10) { band.height = 16; band.y = r.y + r.height - band.height; }
        else if (i <= 19) { band.width = 16; }
        else if (i <= 30) { band.height = 16; }
        else { band.width = 16; band.x = r.x + r.width - band.width; }
        DrawRectangleRec(band,groupColor(tile.colorGroup));

        if (selected) {
            DrawRectangleLinesEx(r,4,kAccent);
        }
        DrawText(tile.code.c_str(),static_cast<int>(r.x + 5),static_cast<int>(r.y + 5),16,kText);
        DrawText(tile.name.c_str(),static_cast<int>(r.x + 5),static_cast<int>(r.y + 25),10,kSubtext);

        if (tile.ownerIndex >= 0) {
            DrawCircle(static_cast<int>(r.x + r.width - 14),static_cast<int>(r.y + 14),7,kTokens[static_cast<std::size_t>(tile.ownerIndex) % kTokens.size()]);
        }
        if (tile.status == PropertyStatusView::Mortgaged) {
            DrawText("G",static_cast<int>(r.x + r.width - 22),static_cast<int>(r.y + 26),14,kDanger);
        }
        for (int h = 0; h < tile.houses && h < 4; ++h) {
            DrawRectangle(static_cast<int>(r.x + 7 + h * 11),static_cast<int>(r.y + r.height - 34),8,8,kAccentAlt);
        }
        if (tile.hotel) {
            DrawRectangle(static_cast<int>(r.x + 7),static_cast<int>(r.y + r.height - 34),16,10,kDanger);
        }
        if (tile.festivalMultiplier > 1) {
            std::string text = "x" + std::to_string(tile.festivalMultiplier);
            DrawText(text.c_str(),static_cast<int>(r.x + r.width - 26),static_cast<int>(r.y + r.height - 22),13,{180,50,200,255});
        }
    }

    drawCenterPanel(boardRect);

    for (std::size_t i = 0; i < tokenPositions.size(); ++i) {
        float bob = std::sin(tokenPhases[i] * 2.0f) * 4.0f;
        Vector2 pos{tokenPositions[i].x, tokenPositions[i].y + bob};
        Color tint = kTokens[i % kTokens.size()];
        DrawCircleLines(static_cast<int>(pos.x),static_cast<int>(pos.y),14 + std::sin(tokenPhases[i] * 2.5f) * 2.0f,Fade(tint,0.3f));
        DrawCircle(static_cast<int>(pos.x),static_cast<int>(pos.y),10,tint);
        DrawText(std::to_string(static_cast<int>(i) + 1).c_str(),static_cast<int>(pos.x - 4),static_cast<int>(pos.y - 6),12,kText);
    }
}

void InGameScene::drawSidebar(const Rectangle& sidebarRect) {
    const GameViewModel& vm = gameFacade->getViewModel();
    DrawRectangleRounded({sidebarRect.x + 4, sidebarRect.y + 7, sidebarRect.width, sidebarRect.height},0.04f,10,Fade(kText,0.07f));
    DrawRectangleRounded(sidebarRect,0.04f,10,Fade(kPanel,0.97f));
    DrawRectangleRoundedLinesEx(sidebarRect,0.04f,10,2,Fade(kPanelBorder,0.8f));

    DrawText("Giliran Sekarang",static_cast<int>(sidebarRect.x + 16),static_cast<int>(sidebarRect.y + 16),24,kText);
    if (!vm.players.empty() && vm.currentPlayerIndex < static_cast<int>(vm.players.size())) {
        const PlayerViewData& current = vm.players[static_cast<std::size_t>(vm.currentPlayerIndex)];
        DrawCircle(static_cast<int>(sidebarRect.x + 28),static_cast<int>(sidebarRect.y + 66),11,kTokens[static_cast<std::size_t>(vm.currentPlayerIndex) % kTokens.size()]);
        DrawText(current.name.c_str(),static_cast<int>(sidebarRect.x + 50),static_cast<int>(sidebarRect.y + 52),26,kText);
        DrawText(("M" + std::to_string(current.money)).c_str(),static_cast<int>(sidebarRect.x + 16),static_cast<int>(sidebarRect.y + 88),22,kAccentAlt);
    }

    if (!vm.board.empty() && vm.selectedTileIndex >= 0 && vm.selectedTileIndex < static_cast<int>(vm.board.size())) {
        const TileViewData& selected = vm.board[static_cast<std::size_t>(vm.selectedTileIndex)];
        DrawText((selected.code + " - " + selected.name).c_str(),static_cast<int>(sidebarRect.x + 16),static_cast<int>(sidebarRect.y + 116),18,kSubtext);
        DrawText(statusText(selected.status).c_str(),static_cast<int>(sidebarRect.x + 16),static_cast<int>(sidebarRect.y + 140),16,kSubtext);
    }

    for (Button& button : actionButtons) button.draw();

    DrawText("Semua Pemain",static_cast<int>(sidebarRect.x + 16),static_cast<int>(sidebarRect.y + 560),22,kText);
    for (std::size_t i = 0; i < vm.players.size(); ++i) {
        float py = sidebarRect.y + 592.0f + i * 28.0f;
        DrawCircle(static_cast<int>(sidebarRect.x + 20),static_cast<int>(py + 8),6,kTokens[i % kTokens.size()]);
        const PlayerViewData& p = vm.players[i];
        std::string row = p.name + " M" + std::to_string(p.money);
        if (p.status != "ACTIVE") row += " [" + p.status + "]";
        DrawText(row.c_str(),static_cast<int>(sidebarRect.x + 36),static_cast<int>(py),17,p.isCurrent ? kText : kSubtext);
    }

    DrawText("Log Terbaru",static_cast<int>(sidebarRect.x + 16),static_cast<int>(sidebarRect.y + 722),22,kText);
    int logCount = std::min<int>(4, static_cast<int>(vm.logs.size()));
    for (int i = 0; i < logCount; ++i) {
        const LogEntryViewData& entry = vm.logs[static_cast<std::size_t>(i)];
        float y = sidebarRect.y + 754.0f + i * 40.0f;
        DrawText(("[T" + std::to_string(entry.turn) + "] " + entry.actor + " | " + entry.type).c_str(),
                 static_cast<int>(sidebarRect.x + 16),static_cast<int>(y),16,kText);
        DrawText(entry.detail.c_str(),static_cast<int>(sidebarRect.x + 16),static_cast<int>(y + 17),15,kSubtext);
    }
}

void InGameScene::drawOverlay(Rectangle screenRect) {
    const GameViewModel& vm = gameFacade->getViewModel();
    if (overlayVisibility <= 0.01f || vm.overlay.type == OverlayType::None) return;

    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Fade(kText,0.38f * overlayVisibility));
    Rectangle panel{screenRect.width * 0.5f - 310.0f,
                    screenRect.height * 0.5f - 252.0f + (1.0f - overlayVisibility) * 24.0f,
                    620.0f,504.0f};
    DrawRectangleRounded({panel.x + 5, panel.y + 9, panel.width, panel.height},0.09f,10,Fade(kText,0.12f * overlayVisibility));
    DrawRectangleRounded(panel,0.09f,10,Fade({250,255,235,255},overlayVisibility));
    DrawRectangleRoundedLinesEx(panel,0.09f,10,2.5f,Fade(kPanelBorder,overlayVisibility));
    DrawText(vm.overlay.title.c_str(),static_cast<int>(panel.x + 22),static_cast<int>(panel.y + 20),32,kText);
    closeOverlayButton.draw();

    float y = panel.y + 80.0f;
    for (const std::string& line : vm.overlay.lines) {
        DrawText(line.c_str(),static_cast<int>(panel.x + 24),static_cast<int>(y),20,kSubtext);
        y += 32.0f;
        if (y > panel.y + panel.height - 70.0f) break;
    }
    if (!vm.overlay.footer.empty()) {
        DrawText(vm.overlay.footer.c_str(),static_cast<int>(panel.x + 24),static_cast<int>(panel.y + panel.height - 42),18,kSubtext);
    }
}

void InGameScene::draw() {
    Rectangle screen{0,0,static_cast<float>(GetScreenWidth()),static_cast<float>(GetScreenHeight())};
    Rectangle boardRect{}, sidebarRect{};
    layoutUi(screen, boardRect, sidebarRect);
    drawBackground(screen);
    drawHeader(screen);
    drawBoard(boardRect);
    drawSidebar(sidebarRect);
    drawOverlay(screen);
}
