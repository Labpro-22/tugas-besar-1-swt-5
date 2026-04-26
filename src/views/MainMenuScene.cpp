#include "../../include/views/MainMenuScene.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include "../../include/coredummy/IGameFacade.hpp"
#include "../../include/views/SceneManager.hpp"

namespace {
// ── Tema Bunga Kuning-Hijau ───────────────────────────────────
const Color kBgTop      {236, 248, 220, 255}; // hijau muda terang
const Color kBgBottom   {255, 248, 195, 255}; // kuning pucat
const Color kPanel      {255, 255, 235, 230}; // putih kekuningan
const Color kPanelBorder{180, 210, 120, 200}; // hijau daun
const Color kAccent     {255, 190,  30, 255}; // kuning bunga matahari
const Color kAccentAlt  { 90, 180,  60, 255}; // hijau daun segar
const Color kText       { 50,  80,  20, 255}; // hijau tua (teks utama)
const Color kSubtext    {100, 130,  50, 255}; // hijau sedang
const Color kMuted      {160, 185, 110, 255}; // hijau muted
const Color kDanger     {220,  80,  60, 255}; // merah untuk quit
const Color kFlower1    {255, 210,  50, 255}; // kelopak kuning
const Color kFlower2    {255, 160,  30, 255}; // kelopak oranye
const Color kStem       {100, 170,  50, 255}; // batang

float easeTowards(float current, float target, float speed) {
    const float c = std::max(0.0f, std::min(speed, 1.0f));
    return current + (target - current) * c;
}

// Gambar bunga kecil dekoratif
void drawFlower(float cx, float cy, float r, float angle, float alpha) {
    // 5 kelopak
    for (int i = 0; i < 5; ++i) {
        float a = angle + i * (2.0f * 3.14159f / 5.0f);
        float px = cx + r * 1.3f * std::cos(a);
        float py = cy + r * 1.3f * std::sin(a);
        DrawCircle(static_cast<int>(px), static_cast<int>(py),
                   r, Fade(kFlower1, alpha));
    }
    // Pusat
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy),
               r * 0.7f, Fade(kAccent, alpha));
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy),
               r * 0.35f, Fade(kFlower2, alpha));
}
}

MainMenuScene::MainMenuScene(SceneManager* manager, IGameFacade* facade)
    : Scene(manager, facade),
      newGameButton("New Game", kAccent, kText),
      loadDemoButton("Demo", kAccentAlt, {255,255,255,255}),
      quitButton("Keluar", kDanger, {255,255,255,255}),
      startGameButton("Mulai!", kAccent, kText),
      cancelButton("Batal", kMuted, kText),
      plusButton("+", kAccentAlt, {255,255,255,255}),
      minusButton("-", kAccentAlt, {255,255,255,255}),
      showNewGameModal(false),
      playerCount(4),
      sceneTime(0.0f),
      modalVisibility(0.0f) {

    playerFields.emplace_back("Nama Pemain 1");
    playerFields.emplace_back("Nama Pemain 2");
    playerFields.emplace_back("Nama Pemain 3");
    playerFields.emplace_back("Nama Pemain 4");

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
        for (int i = 0; i < playerCount; ++i)
            names.push_back(playerFields[static_cast<size_t>(i)].getContent());
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
    const float w = 190.0f;
    const float h = 56.0f;
    const float y = screenRect.y + screenRect.height - 130.0f;
    const float left = screenRect.x + 72.0f;
    const float gap  = 16.0f;
    newGameButton.setBoundary({left,              y, w, h});
    loadDemoButton.setBoundary({left + w + gap,    y, w, h});
    quitButton.setBoundary(   {left + (w+gap)*2,  y, 130.0f, h});
}

void MainMenuScene::update() {
    const float dt = GetFrameTime();
    sceneTime += dt;
    modalVisibility = easeTowards(modalVisibility,
                                   showNewGameModal ? 1.0f : 0.0f, dt * 9.0f);

    Rectangle screenRect{0,0,(float)GetScreenWidth(),(float)GetScreenHeight()};
    layoutButtons(screenRect);

    if (IsKeyPressed(KEY_ESCAPE) && showNewGameModal) showNewGameModal = false;

    if (modalVisibility < 0.02f) {
        newGameButton.update();
        loadDemoButton.update();
        quitButton.update();
        return;
    }
    plusButton.update(); minusButton.update();
    startGameButton.update(); cancelButton.update();
    for (int i = 0; i < playerCount; ++i)
        playerFields[static_cast<size_t>(i)].update();
}

void MainMenuScene::drawBackground(Rectangle screenRect) {
    // Gradient hijau → kuning
    DrawRectangleGradientV(0, 0,
        static_cast<int>(screenRect.width),
        static_cast<int>(screenRect.height),
        kBgTop, kBgBottom);

    // Bunga dekoratif tersebar
    struct FlowerSpec { float x, y, r, phase, speed; };
    const FlowerSpec flowers[] = {
        {0.08f, 0.15f, 28.0f, 0.0f,  0.4f},
        {0.92f, 0.10f, 22.0f, 1.2f,  0.5f},
        {0.05f, 0.75f, 18.0f, 2.1f,  0.35f},
        {0.95f, 0.80f, 24.0f, 0.8f,  0.45f},
        {0.50f, 0.05f, 16.0f, 1.5f,  0.6f},
        {0.20f, 0.92f, 20.0f, 3.0f,  0.3f},
        {0.80f, 0.90f, 14.0f, 0.5f,  0.55f},
        {0.15f, 0.45f, 12.0f, 2.5f,  0.7f},
        {0.88f, 0.50f, 15.0f, 1.8f,  0.42f},
    };
    for (const auto& f : flowers) {
        float angle = sceneTime * f.speed + f.phase;
        float px = screenRect.x + f.x * screenRect.width;
        float py = screenRect.y + f.y * screenRect.height;
        float bob = std::sin(sceneTime * f.speed * 1.5f + f.phase) * 6.0f;
        drawFlower(px, py + bob, f.r, angle, 0.55f);
        // Batang
        DrawLineEx({px, py + bob + f.r}, {px, py + bob + f.r + f.r * 2.5f},
                   2.5f, Fade(kStem, 0.4f));
    }

    // Daun-daun kecil melayang
    for (int i = 0; i < 14; ++i) {
        float x = std::fmod(80.0f + i * 105.0f + sceneTime * (10.0f + i * 0.5f),
                            screenRect.width + 60.0f) - 30.0f;
        float y = 50.0f + std::fmod(i * 67.0f + sceneTime * (6.0f + i * 0.3f),
                                     screenRect.height - 100.0f);
        float s = 5.0f + (i % 3) * 3.0f;
        DrawEllipse(static_cast<int>(x), static_cast<int>(y),
                    s, s * 0.55f, Fade(kAccentAlt, 0.18f));
    }
}

void MainMenuScene::drawHero(Rectangle screenRect) {
    const float floatY = std::sin(sceneTime * 1.1f) * 7.0f;

    // Title
    DrawText("NIMONSPOLI", 72, static_cast<int>(74 + floatY), 76, kText);
    // Underline dekoratif
    DrawRectangleRounded({72, 156.0f + floatY, 480.0f, 8.0f}, 1.0f, 8, kAccent);

    DrawText("Permainan papan strategi khas Indonesia!", 76, 178, 26, kSubtext);
    DrawText("Beli properti, bangun bangunan, dan jadilah orang terkaya!", 76, 210, 22, kMuted);

    // Preview board panel
    Rectangle preview{screenRect.width - 510.0f, 74.0f + floatY, 420.0f, 296.0f};
    // Panel shadow
    DrawRectangleRounded({preview.x+6, preview.y+10, preview.width, preview.height},
                          0.1f, 12, Fade(kText, 0.08f));
    DrawRectangleRounded(preview, 0.1f, 12, Fade(kPanel, 0.96f));
    DrawRectangleRoundedLinesEx(preview, 0.1f, 12, 2.5f, Fade(kPanelBorder, 0.9f));

    // Mini board
    Rectangle board{preview.x + 20.0f, preview.y + 20.0f, 226.0f, 226.0f};
    DrawRectangleRounded(board, 0.04f, 8, {235, 245, 215, 255});
    DrawRectangleRoundedLinesEx(board, 0.04f, 8, 3.0f, kAccentAlt);

    // Grid lines
    float cell = board.width / 11.0f;
    for (int i = 0; i <= 11; ++i) {
        DrawLineEx({board.x + i*cell, board.y},
                   {board.x + i*cell, board.y + board.height},
                   0.8f, Fade(kPanelBorder, 0.4f));
        DrawLineEx({board.x, board.y + i*cell},
                   {board.x + board.width, board.y + i*cell},
                   0.8f, Fade(kPanelBorder, 0.4f));
    }

    // Pita NIMONSPOLI di tengah board
    Rectangle ribbon{board.x + 44.0f, board.y + 74.0f, 138.0f, 72.0f};
    DrawRectanglePro(
        {ribbon.x + ribbon.width*0.5f, ribbon.y + ribbon.height*0.5f, ribbon.width, ribbon.height},
        {ribbon.width*0.5f, ribbon.height*0.5f}, -22.0f, kAccent);
    DrawText("NIMONSPOLI", static_cast<int>(board.x + 50.0f),
             static_cast<int>(board.y + 97.0f), 20, kText);

    // Token pemain animasi
    const float orbit = sceneTime * 1.1f;
    const Color tokenColors[] = {kAccent, kAccentAlt, {255,120,80,255}, {80,160,220,255}};
    for (int i = 0; i < 4; ++i) {
        float px = board.x + 40.0f + i * 44.0f + std::cos(orbit + i) * 5.0f;
        float py = board.y + 185.0f + std::sin(orbit * 1.3f + i * 0.7f) * 8.0f;
        DrawCircle(static_cast<int>(px), static_cast<int>(py), 10.0f, tokenColors[i]);
        DrawCircleLines(static_cast<int>(px), static_cast<int>(py),
                        13.0f + std::sin(orbit + i) * 2.0f, Fade(tokenColors[i], 0.3f));
    }

    // Side panel info
    Rectangle side{preview.x + 260.0f, preview.y + 20.0f, 128.0f, 252.0f};
    DrawRectangleRounded(side, 0.14f, 10, {240, 255, 220, 255});
    DrawText("Fitur", static_cast<int>(side.x+14), static_cast<int>(side.y+14), 22, kText);
    const char* features[] = {"40 petak", "4 pemain", "Lelang", "Festival", "Kartu skill"};
    for (int i = 0; i < 5; ++i)
        DrawText(features[i], static_cast<int>(side.x+14),
                 static_cast<int>(side.y + 50.0f + i * 36.0f), 18, kSubtext);
}

void MainMenuScene::drawFeatureCards(Rectangle screenRect) {
    struct Card { const char* title; const char* lines[2]; Color color; };
    const Card cards[] = {
        {"Properti", {"Beli & bangun", "rumah & hotel"}, kAccent},
        {"Strategi",  {"Gadai, lelang,", "festival sewa"}, kAccentAlt},
        {"Kartu",     {"Skill card &", "efek spesial"}, {255,160,30,255}},
    };

    const float startX = 72.0f;
    const float y      = 420.0f;
    const float gap    = 18.0f;
    const float cardW  = (screenRect.width - startX*2 - gap*2) / 3.0f;

    for (size_t i = 0; i < 3; ++i) {
        float bob = std::sin(sceneTime * 1.4f + i * 0.9f) * 6.0f;
        Rectangle rect{startX + i*(cardW+gap), y + bob, cardW, 158.0f};

        // Shadow
        DrawRectangleRounded({rect.x+4,rect.y+8,rect.width,rect.height}, 0.16f, 10,
                              Fade(kText, 0.07f));
        // Card
        DrawRectangleRounded(rect, 0.16f, 10, Fade(kPanel, 0.95f));
        DrawRectangleRoundedLinesEx(rect, 0.16f, 10, 2.0f, Fade(cards[i].color, 0.6f));

        // Color strip top
        DrawRectangleRounded({rect.x+14, rect.y+14, 80.0f, 10.0f}, 1.0f, 8, cards[i].color);

        // Bunga kecil dekoratif
        drawFlower(rect.x + rect.width - 30.0f, rect.y + 28.0f,
                   9.0f, sceneTime * 0.4f + i, 0.6f);

        DrawText(cards[i].title, static_cast<int>(rect.x+14),
                 static_cast<int>(rect.y+42), 28, kText);
        DrawText(cards[i].lines[0], static_cast<int>(rect.x+14),
                 static_cast<int>(rect.y+92), 20, kSubtext);
        DrawText(cards[i].lines[1], static_cast<int>(rect.x+14),
                 static_cast<int>(rect.y+116), 20, kSubtext);
    }
}

void MainMenuScene::drawNewGameModal(Rectangle screenRect) {
    if (modalVisibility <= 0.01f) return;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  Fade(kText, 0.35f * modalVisibility));

    const float rise = (1.0f - modalVisibility) * 28.0f;
    Rectangle modal{screenRect.width*0.5f-290.0f,
                    screenRect.height*0.5f-234.0f + rise,
                    580.0f, 468.0f};

    DrawRectangleRounded({modal.x+6,modal.y+10,modal.width,modal.height},
                          0.1f, 12, Fade(kText, 0.1f * modalVisibility));
    DrawRectangleRounded(modal, 0.1f, 12, Fade({250,255,235,255}, modalVisibility));
    DrawRectangleRoundedLinesEx(modal, 0.1f, 12, 2.5f, Fade(kPanelBorder, modalVisibility));

    // Bunga sudut modal
    drawFlower(modal.x + modal.width - 30.0f, modal.y + 30.0f,
               16.0f, sceneTime * 0.5f, 0.5f * modalVisibility);
    drawFlower(modal.x + 30.0f, modal.y + 30.0f,
               12.0f, sceneTime * 0.4f + 1.0f, 0.4f * modalVisibility);

    DrawText("Atur Pemain", static_cast<int>(modal.x+26),
             static_cast<int>(modal.y+24), 34, kText);
    DrawText("Masukkan nama pemain (2-4 orang).",
             static_cast<int>(modal.x+26), static_cast<int>(modal.y+68), 20, kSubtext);

    // Player count stepper
    DrawText("Jumlah Pemain", static_cast<int>(modal.x+26),
             static_cast<int>(modal.y+108), 22, kText);
    minusButton.setBoundary({modal.x+196, modal.y+94, 52, 46});
    plusButton.setBoundary( {modal.x+320, modal.y+94, 52, 46});
    DrawRectangleRounded({modal.x+256, modal.y+94, 56, 46}, 0.24f, 8,
                          Fade(kAccent, 0.15f));
    DrawText(std::to_string(playerCount).c_str(),
             static_cast<int>(modal.x+279), static_cast<int>(modal.y+105), 24, kText);
    plusButton.draw(); minusButton.draw();

    // Player name fields
    for (int i = 0; i < 4; ++i) {
        float fieldY = modal.y + 162.0f + i * 60.0f;
        playerFields[static_cast<size_t>(i)].setBoundary(
            {modal.x+26, fieldY, modal.width-52, 48});
        playerFields[static_cast<size_t>(i)].draw();
        if (i >= playerCount) {
            DrawRectangleRounded({modal.x+26, fieldY, modal.width-52, 48},
                                  0.22f, 8, Fade(kText, 0.3f));
            DrawText("tidak digunakan",
                     static_cast<int>(modal.x + modal.width - 180.0f),
                     static_cast<int>(fieldY + 14.0f), 18, Fade(kMuted, 0.9f));
        }
    }

    startGameButton.setBoundary({modal.x+modal.width-220, modal.y+modal.height-72, 156, 50});
    cancelButton.setBoundary(   {modal.x+modal.width-390, modal.y+modal.height-72, 140, 50});
    startGameButton.draw(); cancelButton.draw();
}

void MainMenuScene::draw() {
    Rectangle screenRect{0,0,(float)GetScreenWidth(),(float)GetScreenHeight()};

    drawBackground(screenRect);
    drawHero(screenRect);
    drawFeatureCards(screenRect);

    // Tagline bawah
    DrawText("Versi Indonesia dari permainan papan klasik dunia.",
             76, static_cast<int>(screenRect.height - 208.0f), 24, kText);
    DrawText("Strategi, keberuntungan, dan kecerdasan finansial — semua ada di sini.",
             76, static_cast<int>(screenRect.height - 172.0f), 22, kSubtext);

    // Tombol utama
    newGameButton.draw();
    loadDemoButton.draw();
    quitButton.draw();

    drawNewGameModal(screenRect);
}
