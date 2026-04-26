#include "../../include/views/InGameScene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include "../../include/coredummy/GameViewModel.hpp"
#include "../../include/coredummy/IGameFacade.hpp"
#include "../../include/views/SceneManager.hpp"

namespace {
// ── Tema Bunga Kuning-Hijau ───────────────────────────────────
const Color kBgA        {230, 245, 210, 255}; // hijau muda
const Color kBgB        {255, 248, 195, 255}; // kuning pucat
const Color kPanel      {245, 255, 230, 230};
const Color kPanelBorder{160, 200, 100, 180};
const Color kBoardSurf  {240, 248, 220, 255};
const Color kCenterSurf {225, 242, 200, 255};
const Color kText       { 45,  75,  15, 255};
const Color kSubtext    { 90, 130,  45, 255};
const Color kAccent     {255, 190,  30, 255}; // kuning
const Color kAccentAlt  { 80, 175,  50, 255}; // hijau
const Color kDanger     {210,  70,  50, 255};
const std::array<Color,4> kTokens = {
    Color{255,190, 30,255},  // kuning
    Color{ 80,175, 50,255},  // hijau
    Color{255,120, 60,255},  // oranye
    Color{ 60,160,220,255},  // biru
};

Color groupColor(const std::string& g) {
    if (g == "COKLAT")    return {135, 86, 58, 255};
    if (g == "BIRU_MUDA") return {124,215,255, 255};
    if (g == "MERAH_MUDA")return {245,120,182, 255};
    if (g == "ORANGE")    return {244,154, 74, 255};
    if (g == "MERAH")     return {228, 77, 75, 255};
    if (g == "KUNING")    return {241,213, 81, 255};
    if (g == "HIJAU")     return { 88,191,120, 255};
    if (g == "BIRU_TUA")  return { 65, 92,202, 255};
    if (g == "ABU" || g == "ABU_ABU") return {149,158,176,255};
    return {200,210,185,255};
}

std::string kindLabel(TileKind k) {
    switch (k) {
        case TileKind::Street:   return "Lahan";
        case TileKind::Railroad: return "Stasiun";
        case TileKind::Utility:  return "Utilitas";
        case TileKind::Card:     return "Kartu";
        case TileKind::Festival: return "Festival";
        case TileKind::Tax:      return "Pajak";
        case TileKind::Special:  return "Spesial";
    }
    return "Petak";
}

float ease(float c, float t, float r) {
    float cl = std::max(0.0f, std::min(r, 1.0f));
    return c + (t - c) * cl;
}

void drawSmallFlower(float cx, float cy, float r, float angle, float alpha) {
    for (int i = 0; i < 5; ++i) {
        float a = angle + i * (2.0f * 3.14159f / 5.0f);
        DrawCircle(static_cast<int>(cx + r*1.3f*std::cos(a)),
                   static_cast<int>(cy + r*1.3f*std::sin(a)),
                   r, Fade(kAccent, alpha));
    }
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy),
               r*0.6f, Fade({255,160,30,255}, alpha));
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
        {"Info Petak",  [this]() { gameFacade->openSelectedTileDetails(); }},
        {"Properti",    [this]() { gameFacade->showCurrentPlayerProperties(); }},
        {"Kartu",       [this]() { gameFacade->showCardPopup(); }},
        {"Festival",    [this]() { gameFacade->showFestivalPopup(); }},
        {"Pajak",       [this]() { gameFacade->showTaxPopup(); }},
        {"Lelang",      [this]() { gameFacade->showAuctionPopup(); }},
        {"Simpan",      [this]() { gameFacade->showSavePopup(); }},
        {"Akhir Giliran",[this](){ gameFacade->advanceTurn(); }},
        {"Kemenangan",  [this]() { gameFacade->showVictoryPopup(); }},
    };

    for (const auto& s : specs) {
        Button b(s.first, kAccentAlt, {255,255,255,255});
        b.setOnClick(s.second);
        actionButtons.push_back(b);
    }

    closeOverlayButton.setOnClick([this]() { gameFacade->closeOverlay(); });
    backToMenuButton.setOnClick([this]() { sceneManager->setScene(SceneType::MainMenu); });
}

void InGameScene::onEnter() {
    sceneTime      = 0.0f;
    overlayVisibility = 0.0f;
    tokenPositions.clear();
    tokenPhases.clear();
}

void InGameScene::layoutUi(Rectangle screenRect, Rectangle& boardRect, Rectangle& sidebarRect) {
    const float sideMargin  = 20.0f;
    const float topMargin   = 88.0f;
    const float gap         = 16.0f;
    const float sidebarW    = std::max(330.0f, std::min(screenRect.width * 0.28f, 380.0f));
    const float boardSize   = std::min(screenRect.height - topMargin - 24.0f,
                                        screenRect.width - sidebarW - gap - sideMargin*2.0f);

    boardRect   = {sideMargin, topMargin, boardSize, boardSize};
    sidebarRect = {boardRect.x + boardRect.width + gap, topMargin, sidebarW, boardSize};
}

Rectangle InGameScene::getTileRect(const Rectangle& boardRect, int index) const {
    const float cell = boardRect.width / 11.0f;
    if (index >= 0  && index <= 10)
        return {boardRect.x + boardRect.width - (index+1.0f)*cell,
                boardRect.y + boardRect.height - cell, cell, cell};
    if (index >= 11 && index <= 19)
        return {boardRect.x,
                boardRect.y + boardRect.height - (index-10+1.0f)*cell, cell, cell};
    if (index >= 20 && index <= 30)
        return {boardRect.x + (index-20.0f)*cell, boardRect.y, cell, cell};
    return {boardRect.x + boardRect.width - cell,
            boardRect.y + (index-30.0f)*cell, cell, cell};
}

Vector2 InGameScene::getTileCenter(const Rectangle& boardRect, int index) const {
    auto r = getTileRect(boardRect, index);
    return {r.x + r.width*0.5f, r.y + r.height*0.5f};
}

void InGameScene::updateAnimations(const Rectangle& boardRect) {
    const GameViewModel& vm = gameFacade->getViewModel();
    const size_t pc = vm.players.size();

    if (tokenPositions.size() != pc) {
        tokenPositions.assign(pc, {0,0});
        tokenPhases.assign(pc, 0);
        for (size_t i = 0; i < pc; ++i) {
            tokenPositions[i] = getTileCenter(boardRect, vm.players[i].position);
            tokenPhases[i] = static_cast<float>(i) * 1.37f;
        }
    }

    float dt = GetFrameTime();
    for (size_t i = 0; i < pc; ++i) {
        Vector2 target = getTileCenter(boardRect, vm.players[i].position);
        tokenPositions[i].x = ease(tokenPositions[i].x, target.x, dt * 6.0f);
        tokenPositions[i].y = ease(tokenPositions[i].y, target.y, dt * 6.0f);
        tokenPhases[i] += dt * (1.1f + i * 0.12f);
    }

    const GameViewModel& vmr = gameFacade->getViewModel();
    overlayVisibility = ease(overlayVisibility,
                             vmr.overlay.type == OverlayType::None ? 0.0f : 1.0f,
                             dt * 8.0f);
}

void InGameScene::updateBoardSelection() {
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;
    Vector2 mouse = GetMousePosition();
    for (size_t i = 0; i < tileRects.size(); ++i) {
        if (CheckCollisionPointRec(mouse, tileRects[i])) {
            gameFacade->selectTile(static_cast<int>(i));
            break;
        }
    }
}

void InGameScene::update() {
    sceneTime += GetFrameTime();

    Rectangle screenRect{0,0,(float)GetScreenWidth(),(float)GetScreenHeight()};
    Rectangle boardRect{}, sidebarRect{};
    layoutUi(screenRect, boardRect, sidebarRect);

    tileRects.clear();
    tileRects.reserve(40);
    for (int i = 0; i < 40; ++i) tileRects.push_back(getTileRect(boardRect, i));

    updateAnimations(boardRect);

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (gameFacade->getViewModel().overlay.type != OverlayType::None)
            gameFacade->closeOverlay();
        else {
            sceneManager->setScene(SceneType::MainMenu);
            return;
        }
    }

    updateBoardSelection();

    backToMenuButton.setBoundary({screenRect.width-130.0f, 22.0f, 106.0f, 42.0f});
    backToMenuButton.update();

    float btnW = (sidebarRect.width - 14.0f) * 0.5f;
    float btnH = 44.0f;
    float startY = sidebarRect.y + 238.0f;
    for (size_t i = 0; i < actionButtons.size(); ++i) {
        int row = static_cast<int>(i) / 2;
        int col = static_cast<int>(i) % 2;
        actionButtons[i].setBoundary({
            sidebarRect.x + col*(btnW+14.0f),
            startY + row*54.0f, btnW, btnH});
        actionButtons[i].update();
    }

    if (overlayVisibility > 0.01f) {
        closeOverlayButton.setBoundary({
            screenRect.width*0.5f + 268.0f,
            screenRect.height*0.5f - 240.0f, 50.0f, 38.0f});
        closeOverlayButton.update();
    }
}

void InGameScene::drawBackground(Rectangle screenRect) {
    DrawRectangleGradientV(0, 0,
        static_cast<int>(screenRect.width),
        static_cast<int>(screenRect.height),
        kBgA, kBgB);

    // Bunga sudut
    drawSmallFlower(40,  40,  18, sceneTime*0.3f,        0.3f);
    drawSmallFlower(screenRect.width-50, 50, 14, sceneTime*0.35f+1.0f, 0.25f);
    drawSmallFlower(30,  screenRect.height-50, 12, sceneTime*0.4f+2.0f, 0.25f);
    drawSmallFlower(screenRect.width-40, screenRect.height-40, 16, sceneTime*0.3f+0.5f, 0.25f);

    // Daun melayang
    for (int i = 0; i < 10; ++i) {
        float x = std::fmod(60.0f + i*143.0f + sceneTime*(8+i*0.4f), screenRect.width+50)-25;
        float y = 40.0f + std::fmod(i*73.0f + sceneTime*(5+i*0.25f), screenRect.height-80);
        DrawEllipse(static_cast<int>(x), static_cast<int>(y),
                    4+i%3, 2+i%2, Fade(kAccentAlt, 0.15f));
    }
}

void InGameScene::drawHeader(Rectangle) {
    const GameViewModel& vm = gameFacade->getViewModel();
    DrawText("Nimonspoli", 22, 16, 34, kText);
    DrawText(vm.statusLine.c_str(), 22, 52, 20, kSubtext);

    Rectangle badge{380.0f, 18.0f, 240.0f, 46.0f};
    DrawRectangleRounded(badge, 0.28f, 8, Fade(kAccent, 0.2f));
    DrawRectangleRoundedLinesEx(badge, 0.28f, 8, 1.5f, Fade(kPanelBorder, 0.8f));
    std::string turnText = "Turn " + std::to_string(vm.currentTurn)
                         + " / " + std::to_string(vm.maxTurn);
    DrawText(turnText.c_str(), static_cast<int>(badge.x+20),
             static_cast<int>(badge.y+12), 22, kText);

    backToMenuButton.draw();
}

void InGameScene::drawCenterPanel(const Rectangle& boardRect) {
    float cell = boardRect.width / 11.0f;
    Rectangle center{boardRect.x+cell, boardRect.y+cell,
                     boardRect.width-2*cell, boardRect.height-2*cell};
    DrawRectangleRounded(center, 0.05f, 10, kCenterSurf);
    DrawRectangleRoundedLinesEx(center, 0.05f, 10, 3.0f, Fade(kAccentAlt, 0.6f));

    // Pita judul
    Rectangle ribbon{center.x+center.width*0.25f, center.y+center.height*0.42f,
                     center.width*0.5f, 68.0f};
    DrawRectanglePro(
        {ribbon.x+ribbon.width*0.5f, ribbon.y+ribbon.height*0.5f, ribbon.width, ribbon.height},
        {ribbon.width*0.5f, ribbon.height*0.5f}, -22.0f, kAccent);
    DrawText("NIMONSPOLI",
             static_cast<int>(center.x+center.width*0.27f),
             static_cast<int>(center.y+center.height*0.43f), 38, kText);

    // Kartu animasi
    float cardOffset = std::sin(sceneTime*1.3f)*6.0f;
    Rectangle cardA{center.x+66, center.y+76.0f+cardOffset, 100, 136};
    Rectangle cardB{center.x+center.width-166, center.y+center.height-216-cardOffset, 100, 136};
    DrawRectangleRounded(cardA, 0.14f, 8, Fade(kAccent, 0.7f));
    DrawRectangleRounded(cardB, 0.14f, 8, Fade(kAccentAlt, 0.7f));
    DrawRectangleRoundedLinesEx(cardA, 0.14f, 8, 1.5f, Fade(kText, 0.15f));
    DrawRectangleRoundedLinesEx(cardB, 0.14f, 8, 1.5f, Fade(kText, 0.15f));
    DrawText("SKILL", static_cast<int>(cardA.x+14), static_cast<int>(cardA.y+52), 24, kText);
    DrawText("CHANCE", static_cast<int>(cardB.x+8), static_cast<int>(cardB.y+52), 20, {255,255,255,255});

    // Bunga tengah papan
    drawSmallFlower(center.x+center.width*0.5f, center.y+center.height*0.72f,
                    20.0f, sceneTime*0.6f, 0.4f);
    DrawCircleLines(static_cast<int>(center.x+center.width*0.5f),
                    static_cast<int>(center.y+center.height*0.5f),
                    90.0f+std::sin(sceneTime*1.4f)*8.0f, Fade(kAccent, 0.12f));
}

void InGameScene::drawBoard(const Rectangle& boardRect) {
    const GameViewModel& vm = gameFacade->getViewModel();

    DrawRectangleRounded({boardRect.x+5,boardRect.y+7,boardRect.width,boardRect.height},
                          0.04f, 10, Fade(kText, 0.08f));
    DrawRectangleRounded(boardRect, 0.04f, 10, kBoardSurf);
    DrawRectangleRoundedLinesEx(boardRect, 0.04f, 10, 3.0f, Fade(kAccentAlt, 0.7f));

    for (size_t i = 0; i < vm.board.size() && i < tileRects.size(); ++i) {
        const TileViewData& tile = vm.board[i];
        Rectangle rect = tileRects[i];
        bool selected = (static_cast<int>(i) == vm.selectedTileIndex);

        DrawRectangleRec(rect, kBoardSurf);
        DrawRectangleLinesEx(rect, 1.0f, Fade(kPanelBorder, 0.5f));

        // Color band sesuai posisi
        Rectangle band = rect;
        if (i <= 10)       { band.height = 16; band.y = rect.y+rect.height-band.height; }
        else if (i <= 19)  { band.width  = 16; }
        else if (i <= 30)  { band.height = 16; }
        else               { band.width  = 16; band.x = rect.x+rect.width-band.width; }
        DrawRectangleRec(band, groupColor(tile.colorGroup));

        if (selected) {
            float pulse = 0.5f + 0.5f*std::sin(sceneTime*3.0f);
            DrawRectangleLinesEx(rect, 3.0f+pulse*2.0f, kAccent);
        }

        DrawText(tile.code.c_str(),
                 static_cast<int>(rect.x+5), static_cast<int>(rect.y+5), 16, kText);
        DrawText(tile.name.c_str(),
                 static_cast<int>(rect.x+5), static_cast<int>(rect.y+25), 10, kSubtext);

        if (tile.ownerIndex >= 0) {
            DrawCircle(static_cast<int>(rect.x+rect.width-14),
                       static_cast<int>(rect.y+14), 7.0f,
                       kTokens[static_cast<size_t>(tile.ownerIndex % 4)]);
        }
        if (tile.status == PropertyStatusView::Mortgaged)
            DrawText("G", static_cast<int>(rect.x+rect.width-22),
                     static_cast<int>(rect.y+26), 14, kDanger);

        for (int h = 0; h < tile.houses && h < 4; ++h)
            DrawRectangle(static_cast<int>(rect.x+7+h*11),
                          static_cast<int>(rect.y+rect.height-34), 8, 8, kAccentAlt);
        if (tile.hotel)
            DrawRectangle(static_cast<int>(rect.x+7),
                          static_cast<int>(rect.y+rect.height-34), 16, 10, kDanger);
        if (tile.festivalMultiplier > 1) {
            std::string fest = "x" + std::to_string(tile.festivalMultiplier);
            DrawText(fest.c_str(),
                     static_cast<int>(rect.x+rect.width-26),
                     static_cast<int>(rect.y+rect.height-22), 13, {180,50,200,255});
        }
    }

    drawCenterPanel(boardRect);

    // Token pemain
    for (size_t i = 0; i < tokenPositions.size(); ++i) {
        float bob = std::sin(tokenPhases[i]*2.0f)*4.0f;
        Vector2 pos{tokenPositions[i].x, tokenPositions[i].y+bob};
        Color tint = kTokens[i%4];
        DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y),
                        14.0f+std::sin(tokenPhases[i]*2.5f)*2.0f, Fade(tint, 0.3f));
        DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 10.0f, tint);
        DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), 10.0f, Fade(WHITE, 0.5f));
        DrawText(std::to_string(static_cast<int>(i)+1).c_str(),
                 static_cast<int>(pos.x-4), static_cast<int>(pos.y-6), 12, kText);
    }
}

void InGameScene::drawSidebar(const Rectangle& sidebarRect) {
    const GameViewModel& vm = gameFacade->getViewModel();
    if (vm.players.empty() || vm.board.empty()) return;

    // Panel shadow + panel
    DrawRectangleRounded({sidebarRect.x+4,sidebarRect.y+7,sidebarRect.width,sidebarRect.height},
                          0.04f, 10, Fade(kText, 0.07f));
    DrawRectangleRounded(sidebarRect, 0.04f, 10, Fade(kPanel, 0.97f));
    DrawRectangleRoundedLinesEx(sidebarRect, 0.04f, 10, 2.0f, Fade(kPanelBorder, 0.8f));

    // Bunga dekoratif pojok sidebar
    drawSmallFlower(sidebarRect.x + sidebarRect.width - 22,
                    sidebarRect.y + 22, 9, sceneTime*0.4f, 0.35f);

    const PlayerViewData& active = vm.players[vm.currentPlayerIndex];
    DrawText("Giliran Sekarang", static_cast<int>(sidebarRect.x+16),
             static_cast<int>(sidebarRect.y+16), 24, kText);

    Color tokenC = kTokens[static_cast<size_t>(vm.currentPlayerIndex % 4)];
    DrawCircle(static_cast<int>(sidebarRect.x+28),
               static_cast<int>(sidebarRect.y+66), 11.0f, tokenC);
    DrawText(active.name.c_str(),
             static_cast<int>(sidebarRect.x+50), static_cast<int>(sidebarRect.y+52),
             26, kText);
    DrawText(("M" + std::to_string(active.money)).c_str(),
             static_cast<int>(sidebarRect.x+16), static_cast<int>(sidebarRect.y+88),
             22, kAccentAlt);
    if (!vm.board.empty()) {
        const TileViewData& posTile = vm.board[static_cast<size_t>(active.position)];
        DrawText((posTile.code + " - " + posTile.name).c_str(),
                 static_cast<int>(sidebarRect.x+16), static_cast<int>(sidebarRect.y+116),
                 18, kSubtext);
    }
    DrawText(vm.footerHint.c_str(),
             static_cast<int>(sidebarRect.x+16), static_cast<int>(sidebarRect.y+148),
             17, Fade(kSubtext, 0.85f));

    // Progress bar turn
    Rectangle wave{sidebarRect.x+16, sidebarRect.y+186, sidebarRect.width-32, 16};
    DrawRectangleRounded(wave, 0.8f, 10, Fade(kPanelBorder, 0.35f));
    float progress = vm.maxTurn > 0 ? (float)vm.currentTurn / vm.maxTurn : 0.5f;
    DrawRectangleRounded({wave.x, wave.y, wave.width*progress, wave.height},
                          0.8f, 10, kAccent);

    // Action buttons
    for (Button& b : actionButtons) b.draw();

    // Selected tile info
    const TileViewData& sel = vm.board[static_cast<size_t>(vm.selectedTileIndex)];
    Rectangle inspect{sidebarRect.x, sidebarRect.y+404, sidebarRect.width, 106};
    DrawRectangleRounded({inspect.x+8, inspect.y, inspect.width-16, inspect.height},
                          0.14f, 8, Fade(kAccent, 0.1f));
    DrawText("Petak Dipilih",
             static_cast<int>(inspect.x+16), static_cast<int>(inspect.y+8), 22, kText);
    DrawText((sel.code + " - " + sel.name).c_str(),
             static_cast<int>(inspect.x+16), static_cast<int>(inspect.y+38), 20, kText);
    DrawText(kindLabel(sel.kind).c_str(),
             static_cast<int>(inspect.x+16), static_cast<int>(inspect.y+66), 18, kSubtext);
    if (sel.kind == TileKind::Street || sel.kind == TileKind::Railroad || sel.kind == TileKind::Utility)
        DrawText(("M" + std::to_string(sel.price)).c_str(),
                 static_cast<int>(inspect.x+140), static_cast<int>(inspect.y+66), 18, kAccentAlt);

    // Daftar semua pemain
    Rectangle pPanel{sidebarRect.x, sidebarRect.y+524, sidebarRect.width, 148};
    DrawText("Semua Pemain",
             static_cast<int>(pPanel.x+16), static_cast<int>(pPanel.y+4), 22, kText);
    for (size_t i = 0; i < vm.players.size(); ++i) {
        float py = pPanel.y + 34.0f + i*28.0f;
        DrawCircle(static_cast<int>(pPanel.x+20), static_cast<int>(py+8), 6,
                   kTokens[i%4]);
        std::string row = vm.players[i].name + " M" + std::to_string(vm.players[i].money)
                        + " | " + vm.players[i].status;
        DrawText(row.c_str(), static_cast<int>(pPanel.x+36), static_cast<int>(py),
                 17, vm.players[i].isCurrent ? kText : kSubtext);
    }

    // Kartu tangan
    Rectangle cPanel{sidebarRect.x, sidebarRect.y+684, sidebarRect.width, 80};
    DrawText("Kartu Kemampuan",
             static_cast<int>(cPanel.x+16), static_cast<int>(cPanel.y+4), 22, kText);
    for (size_t i = 0; i < vm.hand.size(); ++i) {
        float sway = std::sin(sceneTime*1.7f + i)*3.0f;
        Rectangle card{cPanel.x+16+i*100, cPanel.y+32+sway, 88, 38};
        DrawRectangleRounded(card, 0.22f, 8, Fade(kAccent, 0.25f));
        DrawRectangleRoundedLinesEx(card, 0.22f, 8, 1.5f, Fade(kPanelBorder, 0.7f));
        DrawText(vm.hand[i].title.c_str(),
                 static_cast<int>(card.x+7), static_cast<int>(card.y+10), 14, kText);
    }

    // Log
    Rectangle lPanel{sidebarRect.x, sidebarRect.y+776, sidebarRect.width,
                     sidebarRect.height-776};
    DrawText("Log Terbaru",
             static_cast<int>(lPanel.x+16), static_cast<int>(lPanel.y+4), 22, kText);
    int logCount = std::min<int>(3, static_cast<int>(vm.logs.size()));
    for (int i = 0; i < logCount; ++i) {
        const LogEntryViewData& log = vm.logs[static_cast<size_t>(i)];
        float ly = lPanel.y + 32.0f + i*40.0f;
        DrawText(("[T" + std::to_string(log.turn) + "] " + log.actor + " | " + log.type).c_str(),
                 static_cast<int>(lPanel.x+16), static_cast<int>(ly), 16, kText);
        DrawText(log.detail.c_str(),
                 static_cast<int>(lPanel.x+16), static_cast<int>(ly+17), 15, kSubtext);
    }
}

void InGameScene::drawOverlay(Rectangle screenRect) {
    const GameViewModel& vm = gameFacade->getViewModel();
    if (overlayVisibility <= 0.01f || vm.overlay.type == OverlayType::None) return;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  Fade(kText, 0.38f * overlayVisibility));

    float rise = (1.0f - overlayVisibility) * 24.0f;
    Rectangle panel{screenRect.width*0.5f-310.0f,
                    screenRect.height*0.5f-252.0f + rise,
                    620.0f, 504.0f};

    DrawRectangleRounded({panel.x+5,panel.y+9,panel.width,panel.height},
                          0.09f, 10, Fade(kText, 0.12f*overlayVisibility));
    DrawRectangleRounded(panel, 0.09f, 10, Fade({250,255,235,255}, overlayVisibility));
    DrawRectangleRoundedLinesEx(panel, 0.09f, 10, 2.5f,
                                Fade(kPanelBorder, overlayVisibility));

    // Bunga sudut overlay
    drawSmallFlower(panel.x+panel.width-28, panel.y+28,
                    14, sceneTime*0.5f, 0.5f*overlayVisibility);

    DrawText(vm.overlay.title.c_str(),
             static_cast<int>(panel.x+22), static_cast<int>(panel.y+20), 32, kText);
    closeOverlayButton.draw();

    float y = panel.y + 80.0f;
    for (const std::string& line : vm.overlay.lines) {
        DrawText(line.c_str(), static_cast<int>(panel.x+24), static_cast<int>(y), 20, kSubtext);
        y += 32.0f;
        if (y > panel.y + panel.height - 90.0f) break;
    }

    DrawRectangleRounded({panel.x+22, panel.y+panel.height-70, panel.width-44, 46},
                          0.22f, 8, Fade(kAccent, 0.2f));
    DrawText(vm.overlay.footer.c_str(),
             static_cast<int>(panel.x+36), static_cast<int>(panel.y+panel.height-57),
             19, kText);
}

void InGameScene::draw() {
    Rectangle screenRect{0,0,(float)GetScreenWidth(),(float)GetScreenHeight()};
    Rectangle boardRect{}, sidebarRect{};
    layoutUi(screenRect, boardRect, sidebarRect);

    drawBackground(screenRect);
    drawHeader(screenRect);
    drawBoard(boardRect);
    drawSidebar(sidebarRect);
    drawOverlay(screenRect);
}
