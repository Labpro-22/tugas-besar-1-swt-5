#include "../../include/views/MainMenuScene.hpp"
#include "../../include/views/SceneManager.hpp"
#include "../../include/coredummy/IGameFacade.hpp"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <string>
#include <vector>

namespace {
const Color kBgTop{236,248,220,255};
const Color kBgBottom{255,248,195,255};
const Color kPanel{255,255,235,230};
const Color kPanelBorder{180,210,120,200};
const Color kAccent{255,190,30,255};
const Color kAccentAlt{90,180,60,255};
const Color kText{50,80,20,255};
const Color kSubtext{100,130,50,255};
const Color kMuted{160,185,110,255};
const Color kDanger{220,80,60,255};
const Color kFlower{255,210,50,255};

float easeTowards(float current, float target, float speed) {
    float c = std::max(0.0f, std::min(speed, 1.0f));
    return current + (target - current) * c;
}

void drawFlower(float cx, float cy, float r, float angle, float alpha) {
    for (int i = 0; i < 5; ++i) {
        float a = angle + i * (6.28318f / 5.0f);
        DrawCircle(static_cast<int>(cx + r * 1.3f * std::cos(a)),
                   static_cast<int>(cy + r * 1.3f * std::sin(a)),
                   r, Fade(kFlower, alpha));
    }
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy), r * 0.7f, Fade(kAccent, alpha));
    DrawCircle(static_cast<int>(cx), static_cast<int>(cy), r * 0.35f, Fade({255,160,30,255}, alpha));
}

std::string trimCopy(const std::string& value) {
    std::size_t first = 0;
    while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first]))) {
        ++first;
    }
    std::size_t last = value.size();
    while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) {
        --last;
    }
    return value.substr(first, last - first);
}

std::string oneLineCopy(std::string value) {
    for (char& ch : value) {
        if (ch == '\n' || ch == '\r' || ch == '\t') {
            ch = ' ';
        }
    }
    return value;
}

std::string fitText(std::string value, int fontSize, int maxWidth) {
    value = oneLineCopy(value);
    if (MeasureText(value.c_str(), fontSize) <= maxWidth) {
        return value;
    }
    while (!value.empty() && MeasureText((value + "...").c_str(), fontSize) > maxWidth) {
        value.pop_back();
    }
    return value + "...";
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
      configPathField("Folder Config"),
      showNewGameModal(false),
      playerCount(4),
      sceneTime(0.0f),
      modalVisibility(0.0f) {

    playerFields.emplace_back("Nama Pemain 1");
    playerFields.emplace_back("Nama Pemain 2");
    playerFields.emplace_back("Nama Pemain 3");
    playerFields.emplace_back("Nama Pemain 4");
    for (TextField& field : playerFields) {
        field.setMaxLength(8);
    }
    configPathField.setContent("config/basic");
    configPathField.setMaxLength(120);

    newGameButton.setOnClick([this]() {
        showNewGameModal = true;
        formError.clear();
        if (trimCopy(configPathField.getContent()).empty()) {
            configPathField.setContent("config/basic");
        }
    });
    loadDemoButton.setOnClick([this]() {
        gameFacade->loadDemoGame();
        sceneManager->setScene(SceneType::InGame);
    });
    quitButton.setOnClick([]() { CloseWindow(); });
    cancelButton.setOnClick([this]() {
        showNewGameModal = false;
        formError.clear();
    });
    plusButton.setOnClick([this]() {
        playerCount = std::min(4, playerCount + 1);
        formError.clear();
    });
    minusButton.setOnClick([this]() {
        playerCount = std::max(2, playerCount - 1);
        formError.clear();
    });
    startGameButton.setOnClick([this]() {
        std::vector<std::string> names;
        for (int i = 0; i < playerCount; ++i) {
            names.push_back(trimCopy(playerFields[static_cast<std::size_t>(i)].getContent()));
        }
        const std::string configDirectory = trimCopy(configPathField.getContent());
        formError = gameFacade->validateNewGameSettings(names, configDirectory);
        if (!formError.empty()) {
            return;
        }
        if (!gameFacade->startNewGame(names, configDirectory)) {
            formError = oneLineCopy(gameFacade->getViewModel().statusLine);
            if (formError.empty()) {
                formError = "Gagal membuat game baru.";
            }
            return;
        }
        showNewGameModal = false;
        formError.clear();
        sceneManager->setScene(SceneType::InGame);
    });
}

void MainMenuScene::onEnter() {
    sceneTime = 0.0f;
    showNewGameModal = false;
    modalVisibility = 0.0f;
    formError.clear();
}

void MainMenuScene::layoutButtons(Rectangle screenRect) {
    const float w = 190.0f;
    const float h = 56.0f;
    const float y = screenRect.y + screenRect.height - 130.0f;
    const float left = screenRect.x + 72.0f;
    const float gap = 16.0f;
    newGameButton.setBoundary({left, y, w, h});
    loadDemoButton.setBoundary({left + w + gap, y, w, h});
    quitButton.setBoundary({left + (w + gap) * 2.0f, y, 130.0f, h});
}

void MainMenuScene::update() {
    sceneTime += GetFrameTime();
    Rectangle screen{0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
    layoutButtons(screen);

    modalVisibility = easeTowards(modalVisibility, showNewGameModal ? 1.0f : 0.0f, GetFrameTime() * 9.0f);
    if (IsKeyPressed(KEY_ESCAPE) && showNewGameModal) {
        showNewGameModal = false;
        formError.clear();
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
    configPathField.update();
}

void MainMenuScene::drawBackground(Rectangle screenRect) {
    DrawRectangleGradientV(0, 0, static_cast<int>(screenRect.width), static_cast<int>(screenRect.height),
                           kBgTop, kBgBottom);
    for (int i = 0; i < 9; ++i) {
        float x = screenRect.x + (0.08f + 0.105f * i) * screenRect.width;
        float y = screenRect.y + (0.15f + 0.08f * (i % 5)) * screenRect.height;
        drawFlower(x, y + std::sin(sceneTime * 0.8f + i) * 7.0f, 12.0f + (i % 3) * 5.0f,
                   sceneTime * 0.4f + i, 0.35f);
    }
}

void MainMenuScene::drawHero(Rectangle) {
    float bob = std::sin(sceneTime * 1.1f) * 7.0f;
    DrawText("NIMONSPOLI", 72, static_cast<int>(74 + bob), 76, kText);
    DrawRectangleRounded({72, 156 + bob, 480, 8}, 1.0f, 8, kAccent);
    DrawText("Permainan papan strategi khas Indonesia!", 76, 178, 26, kSubtext);
    DrawText("Beli properti, bangun bangunan, jadilah orang terkaya!", 76, 210, 22, kMuted);
}

void MainMenuScene::drawFeatureCards(Rectangle screenRect) {
    struct Card { const char* title; const char* line1; const char* line2; Color color; };
    const Card cards[] = {
        {"Properti", "Beli & bangun", "rumah & hotel", kAccent},
        {"Strategi", "Gadai, lelang,", "festival sewa", kAccentAlt},
        {"Kartu", "Skill card &", "efek spesial", {255,160,30,255}},
    };
    float sx = 72.0f;
    float y = 420.0f;
    float gap = 18.0f;
    float cw = (screenRect.width - sx * 2.0f - gap * 2.0f) / 3.0f;
    for (int i = 0; i < 3; ++i) {
        Rectangle r{sx + i * (cw + gap), y + std::sin(sceneTime * 1.4f + i) * 6.0f, cw, 158.0f};
        DrawRectangleRounded({r.x + 4, r.y + 8, r.width, r.height}, 0.16f, 10, Fade(kText, 0.07f));
        DrawRectangleRounded(r, 0.16f, 10, Fade(kPanel, 0.95f));
        DrawRectangleRoundedLinesEx(r, 0.16f, 10, 2.0f, Fade(cards[i].color, 0.6f));
        DrawRectangleRounded({r.x + 14, r.y + 14, 80, 10}, 1.0f, 8, cards[i].color);
        DrawText(cards[i].title, static_cast<int>(r.x + 14), static_cast<int>(r.y + 42), 28, kText);
        DrawText(cards[i].line1, static_cast<int>(r.x + 14), static_cast<int>(r.y + 92), 20, kSubtext);
        DrawText(cards[i].line2, static_cast<int>(r.x + 14), static_cast<int>(r.y + 116), 20, kSubtext);
    }
}

void MainMenuScene::drawNewGameModal(Rectangle screenRect) {
    if (modalVisibility <= 0.01f) return;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(kText, 0.35f * modalVisibility));
    Rectangle modal{screenRect.width * 0.5f - 300.0f,
                    screenRect.height * 0.5f - 280.0f + (1.0f - modalVisibility) * 28.0f,
                    600.0f, 560.0f};
    DrawRectangleRounded({modal.x + 6, modal.y + 10, modal.width, modal.height}, 0.1f, 12, Fade(kText, 0.1f));
    DrawRectangleRounded(modal, 0.1f, 12, Fade({250,255,235,255}, modalVisibility));
    DrawRectangleRoundedLinesEx(modal, 0.1f, 12, 2.5f, Fade(kPanelBorder, modalVisibility));
    DrawText("Atur Pemain", static_cast<int>(modal.x + 26), static_cast<int>(modal.y + 24), 34, kText);

    DrawText("Jumlah Pemain:", static_cast<int>(modal.x + 26), static_cast<int>(modal.y + 84), 22, kText);
    minusButton.setBoundary({modal.x + 196, modal.y + 74, 52, 42});
    plusButton.setBoundary({modal.x + 320, modal.y + 74, 52, 42});
    DrawRectangleRounded({modal.x + 256, modal.y + 74, 56, 42}, 0.24f, 8, Fade(kAccent, 0.15f));
    DrawText(std::to_string(playerCount).c_str(), static_cast<int>(modal.x + 279), static_cast<int>(modal.y + 84), 24, kText);
    plusButton.draw();
    minusButton.draw();

    for (int i = 0; i < 4; ++i) {
        float fy = modal.y + 140 + i * 56;
        playerFields[static_cast<std::size_t>(i)].setBoundary({modal.x + 26, fy, modal.width - 52, 44});
        playerFields[static_cast<std::size_t>(i)].draw();
        if (i >= playerCount) {
            DrawRectangleRounded({modal.x + 26, fy, modal.width - 52, 44}, 0.22f, 8, Fade(kText, 0.3f));
            DrawText("tidak digunakan", static_cast<int>(modal.x + modal.width - 180), static_cast<int>(fy + 12), 18, Fade(kMuted, 0.9f));
        }
    }

    DrawText("Folder Config:", static_cast<int>(modal.x + 26), static_cast<int>(modal.y + 374), 20, kText);
    configPathField.setBoundary({modal.x + 26, modal.y + 402, modal.width - 52, 44});
    configPathField.draw();

    if (!formError.empty()) {
        const std::string shownError = fitText(formError, 18, static_cast<int>(modal.width - 52));
        DrawText(shownError.c_str(), static_cast<int>(modal.x + 26), static_cast<int>(modal.y + 456), 18, kDanger);
    }

    startGameButton.setBoundary({modal.x + modal.width - 220, modal.y + modal.height - 70, 156, 50});
    cancelButton.setBoundary({modal.x + modal.width - 390, modal.y + modal.height - 70, 140, 50});
    startGameButton.draw();
    cancelButton.draw();
}

void MainMenuScene::draw() {
    Rectangle screen{0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
    drawBackground(screen);
    drawHero(screen);
    drawFeatureCards(screen);
    DrawText("Versi Indonesia dari permainan papan klasik dunia.", 76, static_cast<int>(screen.height - 208), 24, kText);
    DrawText("Strategi, keberuntungan, dan kecerdasan finansial.", 76, static_cast<int>(screen.height - 172), 22, kSubtext);
    newGameButton.draw();
    loadDemoButton.draw();
    quitButton.draw();
    drawNewGameModal(screen);
}
