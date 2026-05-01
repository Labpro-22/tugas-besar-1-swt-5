#include "../../include/views/MainMenuScene.hpp"
#include "../../include/views/SceneManager.hpp"
#include "../../include/core/GameManager.hpp"
#include "../../include/core/AccountManager.hpp"
#include "../../include/core/Game.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/core/Account.hpp"
#include "../../include/data-layer/ConfigComposer.hpp"
#include "../../include/utils/BoardBuilder.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <string>
#include <vector>
#include <stdexcept>

namespace {
const Color kBgTop          {236,248,220,255};
const Color kBgBottom       {255,248,195,255};
const Color kPanel          {255,255,235,230};
const Color kPanelBorder    {180,210,120,200};
const Color kAccent         {255,190,30,255};
const Color kAccentAlt      {90,180,60,255};
const Color kText           {50,80,20,255};
const Color kSubtext        {100,130,50,255};
const Color kMuted          {160,185,110,255};
const Color kDanger         {220,80,60,255};
const Color kFlower         {255,210,50,255};

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
    while (!value.empty()) {
        std::string testString = value + "...";
        if (MeasureText(testString.c_str(), fontSize) <= maxWidth) {
            break;
        }
        value.pop_back();
    }
    return value + "...";
}
}

MainMenuScene::MainMenuScene(SceneManager* sm, GameManager* gm, AccountManager* am)
    : Scene(sm, gm, am),
      configPathField("config/basic"),
      loadPathField("data/save.txt"),
      usernameField("Username"),
      passwordField("Password"),
      newGameButton("New Game", kAccent, kText),
      loadGameButton("Load Game", kAccentAlt, {255,255,255,255}),
      registerButton("Daftar", {100,180,90,255}, {255,255,255,255}),
      quitButton("Keluar", kDanger, {255,255,255,255}),
      startGameButton("Mulai!", kAccent, kText),
      cancelButton("Batal", kMuted, kText),
      plusButton("+", kAccentAlt, {255,255,255,255}),
      minusButton("-", kAccentAlt, {255,255,255,255}),
      confirmLoadButton("Muat", kAccent, kText),
      cancelLoadButton("Batal", kMuted, kText),
      confirmRegisterButton("Daftar", kAccent, kText),
      cancelRegisterButton("Batal", kMuted, kText),
      setupModalPopup(600.0f, 560.0f),
      loadModalPopup(600.0f, 340.0f),
      registerModalPopup(500.0f, 380.0f),
      playerCount(4),
      sceneTime(0.0f) {

    setupModalPopup.setTitle("Atur Pemain");
    setupModalPopup.setAnimationSpeed(9.0f);
    setupModalPopup.setShowOverlay(true);

    loadModalPopup.setTitle("Load Game");
    loadModalPopup.setAnimationSpeed(9.0f);
    loadModalPopup.setShowOverlay(true);

    registerModalPopup.setTitle("Daftar Akun");
    registerModalPopup.setAnimationSpeed(9.0f);
    registerModalPopup.setShowOverlay(true);

    playerFields.emplace_back("Nama Pemain 1");
    playerFields.emplace_back("Nama Pemain 2");
    playerFields.emplace_back("Nama Pemain 3");
    playerFields.emplace_back("Nama Pemain 4");
    passwordFields.emplace_back("Password Pemain 1");
    passwordFields.emplace_back("Password Pemain 2");
    passwordFields.emplace_back("Password Pemain 3");
    passwordFields.emplace_back("Password Pemain 4");

    for (TextField& field : playerFields) {
        field.setMaxLength(8);
    }

    configPathField.setContent("config/basic");
    configPathField.setMaxLength(120);

    loadPathField.setContent("data/save.txt");
    loadPathField.setMaxLength(160);

    usernameField.setMaxLength(8);
    passwordField.setMaxLength(32);

    newGameButton.setOnClick([this]() {
        setupModalPopup.setVisible(true);
        loadModalPopup.setVisible(false);
        registerModalPopup.setVisible(false);
        formError.clear();

        if (trimCopy(configPathField.getContent()).empty()) {
            configPathField.setContent("config/basic");
        }
    });

    loadGameButton.setOnClick([this]() {
        loadModalPopup.setVisible(true);
        setupModalPopup.setVisible(false);
        registerModalPopup.setVisible(false);
        loadError.clear();

        if (trimCopy(loadPathField.getContent()).empty()) {
            loadPathField.setContent("data/save.txt");
        }
    });

    registerButton.setOnClick([this]() {
        registerModalPopup.setVisible(true);
        setupModalPopup.setVisible(false);
        loadModalPopup.setVisible(false);
        registerError.clear();
        usernameField.setContent("");
        passwordField.setContent("");
    });

    quitButton.setOnClick([]() {
        CloseWindow();
    });

    cancelButton.setOnClick([this]() {
        setupModalPopup.setVisible(false);
        formError.clear();
    });

    cancelLoadButton.setOnClick([this]() {
        loadModalPopup.setVisible(false);
        loadError.clear();
    });

    cancelRegisterButton.setOnClick([this]() {
        registerModalPopup.setVisible(false);
        registerError.clear();
    });

    confirmLoadButton.setOnClick([this]() {
        onLoadGame();
    });

    confirmRegisterButton.setOnClick([this]() {
        onRegister();
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
        onStartGame();
    });
}

void MainMenuScene::onStartGame() {
    std::vector<std::string> names;
    names.reserve(static_cast<std::size_t>(playerCount));
    std::vector<std::string> passwords;
    passwords.reserve(static_cast<std::size_t>(playerCount));

    for (int i = 0; i < playerCount; ++i) {
        names.push_back(trimCopy(playerFields[static_cast<std::size_t>(i)].getContent()));
        passwords.push_back(trimCopy(passwordFields[static_cast<std::size_t>(i)].getContent()));
    }

    const std::string configDirectory = trimCopy(configPathField.getContent());

    if (configDirectory.empty()) {
        formError = "Folder config tidak boleh kosong.";
        setupModalPopup.setVisible(true);
        return;
    }
    
    std::vector<Account*> usedAccounts;
    for (int i = 0; i < playerCount; ++i) {
        const std::string& name = names[static_cast<std::size_t>(i)];
        const std::string& pass = passwords[static_cast<std::size_t>(i)];

        if (name.empty()) {
            formError = "Nama pemain " + std::to_string(i + 1) + " tidak boleh kosong.";
            setupModalPopup.setVisible(true);
            return;
        }

        if (pass.empty()) {
            formError = "Password pemain " + std::to_string(i + 1) + " tidak boleh kosong.";
            setupModalPopup.setVisible(true);
            return;
        }

        Account* currentAccount = accountManager->getAccount(name, pass);

        if (currentAccount == nullptr) {
            formError = "Username / password salah";
            setupModalPopup.setVisible(true);
            return;
        }
        
        if (std::find(usedAccounts.begin(), usedAccounts.end(), currentAccount) != usedAccounts.end()) {
            formError = "Akun pemain harus unik.";
            setupModalPopup.setVisible(true);
            return;
        }
        
        usedAccounts.push_back(currentAccount);
    }

    try {
        gameManager->startNewGame(configDirectory);

        Game* game = gameManager->getCurrentGame();
        if (game == nullptr) {
            throw std::runtime_error("Game gagal dibuat.");
        }

        std::vector<Player>& players = game->getPlayers();
        players.clear();

        std::vector<int> turnOrder;
        turnOrder.reserve(static_cast<std::size_t>(playerCount));

        int startMoney = game->getConfig().getMiscConfig(SALDO_AWAL);
        if (startMoney <= 0) {
            startMoney = 1500;
        }

        int maxTurn = game->getConfig().getMiscConfig(MAX_TURN);
        if (maxTurn <= 0) {
            maxTurn = 20;
        }

        for (int i = 0; i < playerCount; ++i) {
            const std::string& username = names[static_cast<std::size_t>(i)];

            Account* account = usedAccounts[static_cast<std::size_t>(i)];
            players.emplace_back(i, account, startMoney);
            turnOrder.push_back(i);
        }

        game->getTurnManager() = TurnManager(turnOrder, maxTurn);

        setupModalPopup.setVisible(false);
        formError.clear();
        sceneManager->setScene(SceneType::InGame);
    } catch (const std::exception& e) {
        formError = std::string("Gagal mulai game: ") + e.what();
        setupModalPopup.setVisible(true);
    }
}

void MainMenuScene::onLoadGame() {
    const std::string filePath = trimCopy(loadPathField.getContent());

    if (filePath.empty()) {
        loadError = "Path save file tidak boleh kosong.";
        loadModalPopup.setVisible(true);
        return;
    }

    try {
        gameManager->loadGame(filePath);

        loadModalPopup.setVisible(false);
        loadError.clear();

        sceneManager->setScene(SceneType::InGame);
    } catch (const std::exception& e) {
        loadError = std::string("Gagal load game: ") + e.what();
        loadModalPopup.setVisible(true);
    }
}

void MainMenuScene::onRegister() {
    const std::string username = trimCopy(usernameField.getContent());
    const std::string password = trimCopy(passwordField.getContent());

    if (username.empty()) {
        registerError = "Username tidak boleh kosong.";
        registerModalPopup.setVisible(true);
        return;
    }

    if (password.empty()) {
        registerError = "Password tidak boleh kosong.";
        registerModalPopup.setVisible(true);
        return;
    }

    if (username.length() > 8) {
        registerError = "Username maksimal 8 karakter.";
        registerModalPopup.setVisible(true);
        return;
    }

    if (accountManager->isUsernameTaken(username)) {
        registerError = "Username sudah digunakan.";
        registerModalPopup.setVisible(true);
        return;
    }

    try {
        accountManager->addAccount(Account(username, password, 0));
        registerModalPopup.setVisible(false);
        registerError.clear();
    } catch (const std::exception& e) {
        registerError = std::string("Gagal membuat akun: ") + e.what();
        registerModalPopup.setVisible(true);
    }
}

void MainMenuScene::onEnter() {
    sceneTime = 0.0f;
    setupModalPopup.setVisible(false);
    loadModalPopup.setVisible(false);
    registerModalPopup.setVisible(false);
    formError.clear();
    loadError.clear();
    registerError.clear();
}

void MainMenuScene::layoutButtons(Rectangle screenRect) {
    const float w = 160.0f;
    const float h = 56.0f;
    const float y = screenRect.y + screenRect.height - 130.0f;
    const float left = screenRect.x + 50.0f;
    const float gap = 12.0f;

    newGameButton.setBoundary({left, y, w, h});
    loadGameButton.setBoundary({left + (w + gap), y, w, h});
    registerButton.setBoundary({left + (w + gap) * 2.0f, y, w, h});
    quitButton.setBoundary({left + (w + gap) * 3.0f, y, 130.0f, h});
}

void MainMenuScene::update() {  
    sceneTime += GetFrameTime();

    Rectangle screen{
        0,
        0,
        static_cast<float>(GetScreenWidth()),
        static_cast<float>(GetScreenHeight())
    };

    layoutButtons(screen);

    // Update popups
    setupModalPopup.update();
    loadModalPopup.update();
    registerModalPopup.update();

    // Center popups on screen
    if (setupModalPopup.getVisibility() >= 0.02f || loadModalPopup.getVisibility() >= 0.02f || registerModalPopup.getVisibility() >= 0.02f) {
        setupModalPopup.centerOnScreen();
        loadModalPopup.centerOnScreen();
        registerModalPopup.centerOnScreen();
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (setupModalPopup.getVisibility() >= 0.02f) {
            setupModalPopup.setVisible(false);
            formError.clear();
            return;
        }

        if (loadModalPopup.getVisibility() >= 0.02f) {
            loadModalPopup.setVisible(false);
            loadError.clear();
            return;
        }

        if (registerModalPopup.getVisibility() >= 0.02f) {
            registerModalPopup.setVisible(false);
            registerError.clear();
            return;
        }
    }

    if (setupModalPopup.getVisibility() < 0.02f && loadModalPopup.getVisibility() < 0.02f && registerModalPopup.getVisibility() < 0.02f) {
        newGameButton.update();
        loadGameButton.update();
        registerButton.update();
        quitButton.update();
        return;
    }

    if (setupModalPopup.getVisibility() >= 0.02f) {
        plusButton.update();
        minusButton.update();
        startGameButton.update();
        cancelButton.update();

        for (int i = 0; i < playerCount; ++i) {
            playerFields[static_cast<std::size_t>(i)].update();
            passwordFields[static_cast<std::size_t>(i)].update();
        }

        configPathField.update();
    }

    if (loadModalPopup.getVisibility() >= 0.02f) {
        loadPathField.update();
        confirmLoadButton.update();
        cancelLoadButton.update();
    }

    if (registerModalPopup.getVisibility() >= 0.02f) {
        usernameField.update();
        passwordField.update();
        confirmRegisterButton.update();
        cancelRegisterButton.update();
    }
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

void MainMenuScene::drawSetupModal(Rectangle sr) {
    if (setupModalPopup.getVisibility() <= 0.01f) return;

    setupModalPopup.draw();

    Rectangle modal = setupModalPopup.getBoundary();
    float vis = setupModalPopup.getVisibility();

    DrawText("Jumlah Pemain:", static_cast<int>(modal.x + 26), static_cast<int>(modal.y + 84), 22, Fade(kText, vis));
    minusButton.setBoundary({modal.x + 196, modal.y + 74, 52, 42});
    plusButton.setBoundary({modal.x + 320, modal.y + 74, 52, 42});
    DrawRectangleRounded({modal.x + 256, modal.y + 74, 56, 42}, 0.24f, 8, Fade(kAccent, 0.15f * vis));
    DrawText(std::to_string(playerCount).c_str(), static_cast<int>(modal.x + 279), static_cast<int>(modal.y + 84), 24, Fade(kText, vis));
    plusButton.draw();
    minusButton.draw();

    for (int i = 0; i < 4; ++i) {
        float fy = modal.y + 140 + i * 56;
        playerFields[static_cast<std::size_t>(i)].setBoundary({modal.x + 26, fy, modal.width/2 - 39, 44});
        playerFields[static_cast<std::size_t>(i)].draw();
        passwordFields[static_cast<std::size_t>(i)].setBoundary({modal.x + modal.width/2 + 13, fy, modal.width/2 - 39, 44});
        passwordFields[static_cast<std::size_t>(i)].draw();
        if (i >= playerCount) {
            DrawRectangleRounded({modal.x + 26, fy, modal.width/2 - 39, 44}, 0.22f, 8, Fade(kText, 0.3f * vis));
            DrawText("x", static_cast<int>(modal.x + modal.width/2 - 26 - 13), static_cast<int>(fy + 12), 18, Fade(kMuted, 0.9f * vis));
            DrawRectangleRounded({modal.x + modal.width/2 + 13, fy, modal.width/2 - 39, 44}, 0.22f, 8, Fade(kText, 0.3f * vis));
            DrawText("x", static_cast<int>(modal.x + modal.width - 26 - 26), static_cast<int>(fy + 12), 18, Fade(kMuted, 0.9f * vis));
        }
    }

    DrawText("Folder Config:", static_cast<int>(modal.x + 26), static_cast<int>(modal.y + 374), 20, Fade(kText, vis));
    configPathField.setBoundary({modal.x + 26, modal.y + 402, modal.width - 52, 44});
    configPathField.draw();

    if (!formError.empty()) {
        const std::string shownError = fitText(formError, 18, static_cast<int>(modal.width - 52));
        DrawText(shownError.c_str(), static_cast<int>(modal.x + 26), static_cast<int>(modal.y + 456), 18, Fade(kDanger, vis));
    }

    startGameButton.setBoundary({modal.x + modal.width - 220, modal.y + modal.height - 70, 156, 50});
    cancelButton.setBoundary({modal.x + modal.width - 390, modal.y + modal.height - 70, 140, 50});
    startGameButton.draw();
    cancelButton.draw();
}

void MainMenuScene::drawLoadModal(Rectangle sr) {
    if (loadModalPopup.getVisibility() <= 0.01f) return;

    loadModalPopup.draw();

    Rectangle modal = loadModalPopup.getBoundary();
    float vis = loadModalPopup.getVisibility();

    DrawText(
        "Path Save File:",
        static_cast<int>(modal.x + 26),
        static_cast<int>(modal.y + 92),
        20,
        Fade(kText, vis)
    );

    loadPathField.setBoundary({
        modal.x + 26,
        modal.y + 122,
        modal.width - 52,
        44
    });

    loadPathField.draw();

    if (!loadError.empty()) {
        const std::string shownError = fitText(
            loadError,
            18,
            static_cast<int>(modal.width - 52)
        );

        DrawText(
            shownError.c_str(),
            static_cast<int>(modal.x + 26),
            static_cast<int>(modal.y + 184),
            18,
            Fade(kDanger, vis)
        );
    }

    confirmLoadButton.setBoundary({
        modal.x + modal.width - 220,
        modal.y + modal.height - 70,
        156,
        50
    });

    cancelLoadButton.setBoundary({
        modal.x + modal.width - 390,
        modal.y + modal.height - 70,
        140,
        50
    });

    confirmLoadButton.draw();
    cancelLoadButton.draw();
}

void MainMenuScene::drawRegisterModal(Rectangle sr) {
    if (registerModalPopup.getVisibility() <= 0.01f) return;

    registerModalPopup.draw();

    Rectangle modal = registerModalPopup.getBoundary();
    float vis = registerModalPopup.getVisibility();

    DrawText(
        "Username:",
        static_cast<int>(modal.x + 26),
        static_cast<int>(modal.y + 84),
        20,
        Fade(kText, vis)
    );

    usernameField.setBoundary({
        modal.x + 26,
        modal.y + 114,
        modal.width - 52,
        44
    });

    usernameField.draw();

    DrawText(
        "Password:",
        static_cast<int>(modal.x + 26),
        static_cast<int>(modal.y + 176),
        20,
        Fade(kText, vis)
    );

    passwordField.setBoundary({
        modal.x + 26,
        modal.y + 206,
        modal.width - 52,
        44
    });

    passwordField.draw();

    if (!registerError.empty()) {
        const std::string shownError = fitText(
            registerError,
            18,
            static_cast<int>(modal.width - 52)
        );

        DrawText(
            shownError.c_str(),
            static_cast<int>(modal.x + 26),
            static_cast<int>(modal.y + 260),
            18,
            Fade(kDanger, vis)
        );
    }

    confirmRegisterButton.setBoundary({
        modal.x + modal.width - 220,
        modal.y + modal.height - 70,
        156,
        50
    });

    cancelRegisterButton.setBoundary({
        modal.x + modal.width - 390,
        modal.y + modal.height - 70,
        140,
        50
    });

    confirmRegisterButton.draw();
    cancelRegisterButton.draw();
}

void MainMenuScene::draw() {
    Rectangle screen{0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())};
    drawBackground(screen);
    drawHero(screen);
    drawFeatureCards(screen);
    DrawText("Versi Indonesia dari permainan papan klasik dunia.", 76, static_cast<int>(screen.height - 208), 24, kText);
    DrawText("Strategi, keberuntungan, dan kecerdasan finansial.", 76, static_cast<int>(screen.height - 172), 22, kSubtext);

    newGameButton.draw();
    loadGameButton.draw();
    registerButton.draw();
    quitButton.draw();

    drawSetupModal(screen);
    drawLoadModal(screen);
    drawRegisterModal(screen);
}
