#include "../../include/views/InGameScene.hpp"
#include "../../include/views/SceneManager.hpp"
#include "../../include/core/GameManager.hpp"
#include "../../include/core/Game.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/utils/Board.hpp"
#include "../../include/utils/Tile.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/utils/RailroadTile.hpp"
#include "../../include/utils/UtilityTile.hpp"
#include "../../include/utils/LogEntry.hpp"
#include "../../include/core/TurnManager.hpp"
#include "../../include/utils/Logger.hpp"
#include "raylib.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <string>
#include <vector>
#include <cctype>
#include <exception>

enum class PropertyStatusView {
    Bank,
    Owned,
    Mortgaged
};

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
}

InGameScene::InGameScene(SceneManager* sm, GameManager* gm, AccountManager* am)
    : Scene(sm, gm, am),
      closeOverlayBtn("X", kDanger, {255,255,255,255}),
      backToMenuBtn("Menu", kSubtext, {255,255,255,255}),
      saveConfirmButton("Simpan", kAccent, kText),
      saveCancelButton("Batal", kPanelBorder, kText),
      diceRollButton("Lempar Dadu", kAccentAlt, {255,255,255,255}),
      diceManualButton("Atur Dadu", kAccent, kText),
      diceManualConfirmButton("Gunakan", kAccentAlt, {255,255,255,255}),
      diceBackButton("Kembali", kPanelBorder, kText),
      diceCancelButton("Batal", kDanger, {255,255,255,255}),
      savePathField("data/save.txt"),
      diceOneField("1-6"),
      diceTwoField("1-6"),
      sceneTime(0),
      selectedTile(0),
      overlayOpen(false),
      overlayVis(0),
      showSaveModal(false),
      saveModalVis(0),
      showDiceModal(false),
      diceManualMode(false),
      diceModalVis(0) {

    auto selectedPropertyCode = [this](Game* g, std::string& code) -> bool {
        if (g == nullptr || g->getBoard().size() == 0) {
            return false;
        }

        if (selectedTile < 0 || selectedTile >= g->getBoard().size()) {
            return false;
        }

        Tile* tile = g->getBoard().getTileByIndex(selectedTile);
        PropertyTile* property = dynamic_cast<PropertyTile*>(tile);

        if (property == nullptr) {
            return false;
        }

        code = property->getCode();
        return true;
    };

    struct Spec {
        const char* label;
        std::function<void()> fn;
    };

    std::vector<Spec> specs = {
        {"Dadu", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr || g->isGameOver()) return;

            if (g->getHasRolledThisTurn()) {
                return;
            }

            showDiceModal = true;
            diceManualMode = false;
            diceError.clear();
            diceOneField.setContent("");
            diceTwoField.setContent("");
            overlayOpen = false;
        }},

        {"Beli", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr || g->isGameOver()) return;

            bool ok = g->buyCurrentProperty();

            showOverlay(
                ok ? "Pembelian Berhasil" : "Pembelian Gagal",
                {
                    ok
                        ? "Properti berhasil dibeli."
                        : "Tidak ada properti bank yang bisa dibeli atau uang tidak cukup."
                }
            );
        }},

        {"Gadai", [this, selectedPropertyCode]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr || g->isGameOver()) return;

            std::string code;
            if (!selectedPropertyCode(g, code)) {
                showOverlay("Gadai Gagal", {"Pilih petak properti terlebih dahulu."});
                return;
            }

            bool ok = g->mortgageProperty(code);

            showOverlay(
                ok ? "Gadai Berhasil" : "Gadai Gagal",
                {
                    ok
                        ? "Properti " + code + " berhasil digadaikan."
                        : "Pilih properti milik pemain aktif yang belum digadaikan."
                }
            );
        }},

        {"Tebus", [this, selectedPropertyCode]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr || g->isGameOver()) return;

            std::string code;
            if (!selectedPropertyCode(g, code)) {
                showOverlay("Tebus Gagal", {"Pilih petak properti terlebih dahulu."});
                return;
            }

            bool ok = g->redeemProperty(code);

            showOverlay(
                ok ? "Tebus Berhasil" : "Tebus Gagal",
                {
                    ok
                        ? "Properti " + code + " berhasil ditebus."
                        : "Pilih properti tergadai milik pemain aktif dan pastikan uang cukup."
                }
            );
        }},

        {"Bangun", [this, selectedPropertyCode]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr || g->isGameOver()) return;

            std::string code;
            if (!selectedPropertyCode(g, code)) {
                showOverlay("Bangun Gagal", {"Pilih petak street/property terlebih dahulu."});
                return;
            }

            bool ok = g->buildProperty(code);

            showOverlay(
                ok ? "Bangun Berhasil" : "Bangun Gagal",
                {
                    ok
                        ? "Bangunan di " + code + " berhasil ditingkatkan."
                        : "Pilih street milik pemain aktif dan pastikan syarat bangun terpenuhi."
                }
            );
        }},

        {"Info Petak", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr || g->getBoard().size() == 0) return;

            Tile* t = g->getBoard().getTileByIndex(selectedTile);
            if (t == nullptr) return;

            std::vector<std::string> lines = {
                "Kode: " + t->getCode(),
                "Nama: " + t->getName()
            };

            PropertyTile* pt = dynamic_cast<PropertyTile*>(t);
            if (pt != nullptr) {
                lines.push_back("Harga: M" + std::to_string(pt->getLandPrice()));
                lines.push_back("Pemilik: " + std::string(pt->getOwner() ? pt->getOwner()->getUsername() : "BANK"));
                lines.push_back("Status: " + std::string(
                    pt->isMortgaged() ? "DIGADAIKAN" :
                    pt->isOwned() ? "DIMILIKI" :
                    "BANK"
                ));

                StreetTile* st = dynamic_cast<StreetTile*>(pt);
                if (st != nullptr) {
                    lines.push_back("Grup: " + st->getColorGroup());
                    lines.push_back(
                        "Rumah: " + std::to_string(st->getHouseCount()) +
                        (st->hasHotelBuilt() ? " (Hotel)" : "")
                    );

                    if (pt->getFestivalMultiplier() > 1) {
                        lines.push_back(
                            "Festival x" + std::to_string(pt->getFestivalMultiplier()) +
                            " (" + std::to_string(pt->getFestivalDuration()) + " giliran)"
                        );
                    }
                }
            }

            showOverlay(t->getName(), lines, "Klik X untuk menutup.");
        }},

        {"Properti", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr) return;

            int idx = g->getTurnManager().getCurrentPlayerIndex();
            if (idx < 0 || idx >= static_cast<int>(g->getPlayers().size())) return;

            Player& p = g->getPlayer(idx);
            std::vector<std::string> lines = {
                "Pemilik: " + p.getUsername()
            };

            for (PropertyTile* pt : p.getOwnedProperties()) {
                std::string row = pt->getCode() + " - " + pt->getName();
                if (pt->isMortgaged()) row += " [GADAI]";
                lines.push_back(row);
            }

            if (lines.size() == 1) {
                lines.push_back("(tidak ada properti)");
            }

            showOverlay("Properti " + p.getUsername(), lines);
        }},

        {"Kartu", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr) return;

            int idx = g->getTurnManager().getCurrentPlayerIndex();
            if (idx < 0 || idx >= static_cast<int>(g->getPlayers().size())) return;

            Player& p = g->getPlayer(idx);
            std::vector<std::string> lines;

            for (const auto& c : p.getHandCards()) {
                lines.push_back(c->getName() + " - " + c->getDescription());
            }

            if (lines.empty()) {
                lines.push_back("(tidak ada kartu kemampuan)");
            }

            showOverlay(
                "Kartu Kemampuan",
                lines,
                "Gunakan command/fitur kartu sesuai implementasi AbilityCard."
            );
        }},

        {"Festival", [this]() {
            showOverlay(
                "Festival",
                {
                    "Mendaratlah di petak Festival untuk mengaktifkan.",
                    "Sewa properti pilihan dapat berlipat selama beberapa giliran.",
                    "Multiplier festival akan ditampilkan pada petak terkait."
                }
            );
        }},

        {"Pajak", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr) return;

            int pph = g->getConfig().getTaxConfig(PPH);
            int pbm = g->getConfig().getTaxConfig(PBM);

            showOverlay(
                "Pajak",
                {
                    "PPH: M" + std::to_string(pph) + " flat atau berdasarkan aturan pajak.",
                    "PBM: M" + std::to_string(pbm) + " flat.",
                    "Pajak akan diproses ketika pemain mendarat di petak pajak."
                }
            );
        }},

        {"Lelang", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr) return;

            if (!g->getAuctionManager().isAuctionActive()) {
                showOverlay(
                    "Lelang",
                    {
                        "Tidak ada lelang aktif.",
                        "Lelang biasanya dimulai ketika pemain menolak membeli properti bank."
                    }
                );
                return;
            }

            Player* bidder = g->getAuctionManager().getCurrentTurnPlayer();

            showOverlay(
                "Lelang Aktif",
                {
                    "Giliran: " + std::string(bidder ? bidder->getUsername() : "?"),
                    "Bid tertinggi: M" + std::to_string(g->getAuctionManager().getHighestBid()),
                    "Gunakan fitur/command tawar sesuai AuctionManager."
                }
            );
        }},

        {"Simpan", [this]() {
            showSaveModal = true;
            overlayOpen = false;
            saveError.clear();

            if (savePathField.getContent().empty()) {
                savePathField.setContent("data/save.txt");
            }
        }},

        {"Akhir Giliran", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr || g->isGameOver()) return;

            g->endTurn();

            showOverlay(
                "Giliran Selesai",
                {
                    "Giliran berpindah ke pemain berikutnya."
                }
            );
        }},

        {"Kemenangan", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr) return;

            std::vector<Player*> sorted;
            for (Player& p : g->getPlayers()) {
                sorted.push_back(&p);
            }

            std::sort(sorted.begin(), sorted.end(), [](Player* a, Player* b) {
                return a->getTotalWealth() > b->getTotalWealth();
            });

            std::vector<std::string> lines;
            lines.push_back("=== Klasemen Sementara ===");

            for (std::size_t i = 0; i < sorted.size(); ++i) {
                lines.push_back(
                    std::to_string(i + 1) + ". " +
                    sorted[i]->getUsername() +
                    " - M" + std::to_string(sorted[i]->getTotalWealth()) +
                    (sorted[i]->isBankrupt() ? " [BANGKRUT]" : "")
                );
            }

            showOverlay(
                "Kemenangan",
                lines,
                "Pemenang final ditentukan berdasarkan kondisi gameOver / max turn."
            );
        }},
    };

    for (auto& s : specs) {
        Button b(s.label, kAccentAlt, {255,255,255,255});
        b.setOnClick(s.fn);
        actionButtons.push_back(b);
    }

    closeOverlayBtn.setOnClick([this]() {
        overlayOpen = false;
    });

    saveConfirmButton.setOnClick([this]() {
        onSaveGame();
    });

    saveCancelButton.setOnClick([this]() {
        showSaveModal = false;
        saveError.clear();
    });

    savePathField.setMaxLength(160);
    savePathField.setContent("data/save.txt");
    diceOneField.setMaxLength(2);
    diceTwoField.setMaxLength(2);

    diceRollButton.setOnClick([this]() {
        showDiceModal = false;
        diceError.clear();
        rollDiceAndShowResult();
    });

    diceManualButton.setOnClick([this]() {
        diceManualMode = true;
        diceError.clear();
        diceOneField.setContent("");
        diceTwoField.setContent("");
    });

    diceManualConfirmButton.setOnClick([this]() {
        onManualDiceSubmit();
    });

    diceBackButton.setOnClick([this]() {
        diceManualMode = false;
        diceError.clear();
    });

    diceCancelButton.setOnClick([this]() {
        showDiceModal = false;
        diceManualMode = false;
        diceError.clear();
    });

    backToMenuBtn.setOnClick([this]() {
        sceneManager->setScene(SceneType::MainMenu);
    });
}

void InGameScene::onEnter() {
    sceneTime = 0;
    overlayVis = 0;
    overlayOpen = false;

    showSaveModal = false;
    saveModalVis = 0;
    saveError.clear();
    showDiceModal = false;
    diceManualMode = false;
    diceModalVis = 0;
    diceError.clear();

    tokenPos.clear();
    tokenPhase.clear();
    selectedTile = 0;
}

void InGameScene::rollDiceAndShowResult() {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || g->isGameOver()) return;

    if (g->getHasRolledThisTurn()) {
        showOverlay("Dadu", {"Dadu sudah digunakan pada giliran ini."});
        return;
    }

    auto result = g->rollDiceForCurrentPlayer();

    if (result.first == 0 && result.second == 0) {
        showOverlay("Dadu", {"Dadu tidak bisa dilempar saat ini."});
        return;
    }

    showOverlay(
        "Dadu",
        {
            "Hasil: " + std::to_string(result.first) + " + " + std::to_string(result.second),
            "Total langkah: " + std::to_string(result.first + result.second)
        }
    );
}

void InGameScene::onManualDiceSubmit() {
    auto parseDie = [](const std::string& text, int& value) -> bool {
        if (text.size() != 1 || !std::isdigit(static_cast<unsigned char>(text[0]))) {
            return false;
        }

        value = text[0] - '0';
        return true;
    };

    int d1 = 0;
    int d2 = 0;

    if (!parseDie(diceOneField.getContent(), d1) || !parseDie(diceTwoField.getContent(), d2)) {
        diceError = "Masukkan angka 1 sampai 6 untuk masing-masing dadu.";
        return;
    }

    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || g->isGameOver()) {
        diceError = "Dadu tidak bisa diatur saat ini.";
        return;
    }

    if (g->getHasRolledThisTurn()) {
        diceError = "Dadu sudah digunakan pada giliran ini.";
        return;
    }

    try {
        g->getDice().setManual(d1, d2);
    } catch (const std::exception& e) {
        diceError = e.what();
        return;
    }

    showDiceModal = false;
    diceManualMode = false;
    diceError.clear();
    rollDiceAndShowResult();
}

void InGameScene::onSaveGame() {
    const std::string filePath = trimCopy(savePathField.getContent());

    if (filePath.empty()) {
        saveError = "Path save file tidak boleh kosong.";
        showSaveModal = true;
        return;
    }

    try {
        gameManager->saveGame(filePath);

        showSaveModal = false;
        saveError.clear();

        showOverlay(
            "Save Berhasil",
            {
                "Game berhasil disimpan.",
                "File: " + filePath
            }
        );
    } catch (const std::exception& e) {
        saveError = std::string("Gagal save game: ") + e.what();
        showSaveModal = true;
    }
}

void InGameScene::showOverlay(
    const std::string& title,
    const std::vector<std::string>& lines,
    const std::string& footer
) {
    overlayTitle = title;
    overlayLines = lines;
    overlayFooter = footer;
    overlayOpen = true;
}

void InGameScene::layoutUi(Rectangle sr,Rectangle& br,Rectangle& sb){
    float sm=20,tm=88,gap=16;
    float sw=std::max(330.f,std::min(sr.width*.28f,380.f));
    float bs=std::min(sr.height-tm-24,sr.width-sw-gap-sm*2);
    br={sm,tm,bs,bs}; sb={br.x+br.width+gap,tm,sw,bs};
}

Rectangle InGameScene::getTileRect(const Rectangle& br,int i) const {
    float c=br.width/11.f;
    if(i>=0&&i<=10)return{br.x+br.width-(i+1.f)*c,br.y+br.height-c,c,c};
    if(i>=11&&i<=19)return{br.x,br.y+br.height-(i-10+1.f)*c,c,c};
    if(i>=20&&i<=30)return{br.x+(i-20.f)*c,br.y,c,c};
    return{br.x+br.width-c,br.y+(i-30.f)*c,c,c};
}
Vector2 InGameScene::getTileCenter(const Rectangle& br,int i) const{auto r=getTileRect(br,i);return{r.x+r.width*.5f,r.y+r.height*.5f};}

void InGameScene::updateAnimations(const Rectangle& br){
    Game* g = gameManager->getCurrentGame(); if(!g)return;
    size_t pc = g->getPlayers().size();
    if(tokenPos.size()!=pc){tokenPos.assign(pc,{0,0});tokenPhase.assign(pc,0);
        for(size_t i=0;i<pc;++i){tokenPos[i]=getTileCenter(br,g->getPlayers()[i].getPosition());tokenPhase[i]=i*1.37f;}}
    float dt=GetFrameTime();
    for(size_t i=0;i<pc;++i){
        Vector2 tgt=getTileCenter(br,g->getPlayers()[i].getPosition());
        tokenPos[i].x=ease(tokenPos[i].x,tgt.x,dt*6);tokenPos[i].y=ease(tokenPos[i].y,tgt.y,dt*6);
        tokenPhase[i]+=dt*(1.1f+i*.12f);}
    overlayVis=ease(overlayVis,overlayOpen?1.f:0.f,dt*8);
    saveModalVis = ease(saveModalVis, showSaveModal ? 1.f : 0.f, dt * 8);
    diceModalVis = ease(diceModalVis, showDiceModal ? 1.f : 0.f, dt * 8);
}

void InGameScene::update(){
    sceneTime+=GetFrameTime();
    Rectangle sr{0,0,(float)GetScreenWidth(),(float)GetScreenHeight()},br{},sb{};
    layoutUi(sr,br,sb);
    tileRects.clear();
    Game* g = gameManager->getCurrentGame();
    int tileCount = g ? g->getBoard().size() : 0;
    for(int i=0;i<tileCount;++i) tileRects.push_back(getTileRect(br,i));
    updateAnimations(br);
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (showDiceModal) {
            showDiceModal = false;
            diceManualMode = false;
            diceError.clear();
            return;
        }

        if (showSaveModal) {
            showSaveModal = false;
            saveError.clear();
            return;
        }

        if (overlayOpen) {
            overlayOpen = false;
        } else {
            sceneManager->setScene(SceneType::MainMenu);
            return;
        }
    }

    if (saveModalVis < .02f && diceModalVis < .02f && overlayVis < .02f && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();

        for (size_t i = 0; i < tileRects.size(); ++i) {
            if (CheckCollisionPointRec(m, tileRects[i])) {
                selectedTile = static_cast<int>(i);
                break;
            }
        }
    }

    backToMenuBtn.setBoundary({sr.width - 130, 22, 106, 42});

    if (saveModalVis < .02f && diceModalVis < .02f && overlayVis < .02f) {
        backToMenuBtn.update();
    }

    float bw = (sb.width - 14) * .5f;
    float bh = 44;
    float sy = sb.y + 238;
    bool diceButtonDisabled = false;

    if (g != nullptr) {
        diceButtonDisabled = g->isGameOver() || g->getHasRolledThisTurn();
    }

    for (size_t i = 0; i < actionButtons.size(); ++i) {
        int r = static_cast<int>(i) / 2;
        int c = static_cast<int>(i) % 2;

        if (i == 0) {
            actionButtons[i].disabled = diceButtonDisabled;
        }

        actionButtons[i].setBoundary({
            sb.x + c * (bw + 14),
            sy + r * 54,
            bw,
            bh
        });

        if (saveModalVis < .02f && diceModalVis < .02f && overlayVis < .02f) {
            actionButtons[i].update();
        }
    }

    if (overlayVis > .01f) {
        closeOverlayBtn.setBoundary({
            sr.width * .5f + 268,
            sr.height * .5f - 240,
            50,
            38
        });

        closeOverlayBtn.update();
    }

    if (saveModalVis > .01f) {
        Rectangle p{
            sr.width * .5f - 300,
            sr.height * .5f - 170 + (1 - saveModalVis) * 24,
            600,
            340
        };

        savePathField.setBoundary({
            p.x + 24,
            p.y + 122,
            p.width - 48,
            44
        });

        saveConfirmButton.setBoundary({
            p.x + p.width - 220,
            p.y + p.height - 70,
            156,
            50
        });

        saveCancelButton.setBoundary({
            p.x + p.width - 390,
            p.y + p.height - 70,
            140,
            50
        });

        savePathField.update();
        saveConfirmButton.update();
        saveCancelButton.update();
    }

    if (diceModalVis > .01f) {
        Rectangle p{
            sr.width * .5f - 280,
            sr.height * .5f - 170 + (1 - diceModalVis) * 24,
            560,
            340
        };

        if (diceManualMode) {
            diceOneField.setBoundary({
                p.x + 80,
                p.y + 132,
                150,
                52
            });

            diceTwoField.setBoundary({
                p.x + p.width - 230,
                p.y + 132,
                150,
                52
            });

            diceManualConfirmButton.setBoundary({
                p.x + p.width - 204,
                p.y + p.height - 72,
                156,
                50
            });

            diceBackButton.setBoundary({
                p.x + 48,
                p.y + p.height - 72,
                140,
                50
            });

            diceOneField.update();
            diceTwoField.update();
            diceManualConfirmButton.update();
            diceBackButton.update();
        } else {
            diceRollButton.setBoundary({
                p.x + 48,
                p.y + 128,
                p.width - 96,
                52
            });

            diceManualButton.setBoundary({
                p.x + 48,
                p.y + 196,
                p.width - 96,
                52
            });

            diceCancelButton.setBoundary({
                p.x + p.width - 188,
                p.y + p.height - 72,
                140,
                50
            });

            diceRollButton.update();
            diceManualButton.update();
            diceCancelButton.update();
        }
    }
}

void InGameScene::drawBackground(Rectangle sr){
    DrawRectangleGradientV(0,0,int(sr.width),int(sr.height),kBgA,kBgB);
    drawSmallFlower(40,40,18,sceneTime*.3f,.3f);
    drawSmallFlower(sr.width-50,50,14,sceneTime*.35f+1,.25f);
    drawSmallFlower(30,sr.height-50,12,sceneTime*.4f+2,.25f);
    drawSmallFlower(sr.width-40,sr.height-40,16,sceneTime*.3f+.5f,.25f);
    for(int i=0;i<10;++i){float x=fmodf(60+i*143+sceneTime*(8+i*.4f),sr.width+50)-25;
        float y=40+fmodf(i*73+sceneTime*(5+i*.25f),sr.height-80);
        DrawEllipse(int(x),int(y),4+i%3,2+i%2,Fade(kAccentAlt,.15f));}
}

void InGameScene::drawHeader(Rectangle sr){
    Game* g = gameManager->getCurrentGame();
    DrawText("Nimonspoli",22,16,34,kText);
    if(g){
        int ci=g->getTurnManager().getCurrentPlayerIndex();
        std::string status = (ci>=0&&ci<(int)g->getPlayers().size()) ? "Giliran: "+g->getPlayers()[ci].getUsername() : "";
        DrawText(status.c_str(),22,52,20,kSubtext);
        Rectangle badge{380,18,240,46};
        DrawRectangleRounded(badge,.28f,8,Fade(kAccent,.2f));
        DrawRectangleRoundedLinesEx(badge,.28f,8,1.5f,Fade(kPanelBorder,.8f));
        std::string tt="Turn "+std::to_string(g->getTurnManager().getCurrentTurn())+" / "+std::to_string(g->getTurnManager().getMaxTurn());
        DrawText(tt.c_str(),int(badge.x+20),int(badge.y+12),22,kText);
    }
    backToMenuBtn.draw();
}

void InGameScene::drawCenterPanel(const Rectangle& br){
    float c=br.width/11.f;
    Rectangle ctr{br.x+c,br.y+c,br.width-2*c,br.height-2*c};
    DrawRectangleRounded(ctr,.05f,10,kCenterSurf);
    DrawRectangleRoundedLinesEx(ctr,.05f,10,3,Fade(kAccentAlt,.6f));
    Rectangle rib{ctr.x+ctr.width*.25f,ctr.y+ctr.height*.42f,ctr.width*.5f,68};
    DrawRectanglePro({rib.x+rib.width*.5f,rib.y+rib.height*.5f,rib.width,rib.height},{rib.width*.5f,rib.height*.5f},-22,kAccent);
    DrawText("NIMONSPOLI",int(ctr.x+ctr.width*.27f),int(ctr.y+ctr.height*.43f),38,kText);
    drawSmallFlower(ctr.x+ctr.width*.5f,ctr.y+ctr.height*.72f,20,sceneTime*.6f,.4f);
}

Color groupCol(const std::string& group) {
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

void InGameScene::drawBoard(const Rectangle& br){
    Game* g = gameManager->getCurrentGame(); if(!g)return;
    DrawRectangleRounded({br.x+5,br.y+7,br.width,br.height},.04f,10,Fade(kText,.08f));
    DrawRectangleRounded(br,.04f,10,kBoardSurf);
    DrawRectangleRoundedLinesEx(br,.04f,10,3,Fade(kAccentAlt,.7f));

    if(g->getBoard().size()==0) { drawCenterPanel(br); return; }
    const auto& tiles = g->getBoard().getTiles();
    for(size_t i=0;i<tiles.size()&&i<tileRects.size();++i){
        Tile* tile=tiles[i]; if(!tile)continue;
        Rectangle r=tileRects[i];
        bool sel=(int(i)==selectedTile);
        DrawRectangleRec(r,kBoardSurf);
        DrawRectangleLinesEx(r,1,Fade(kPanelBorder,.5f));

        // Color band
        std::string cg="DEFAULT";
        StreetTile* st=dynamic_cast<StreetTile*>(tile);
        if(st) cg=st->getColorGroup();
        Rectangle band=r;
        if(i<=10){band.height=16;band.y=r.y+r.height-band.height;}
        else if(i<=19){band.width=16;}
        else if(i<=30){band.height=16;}
        else{band.width=16;band.x=r.x+r.width-band.width;}
        DrawRectangleRec(band,groupCol(cg));

        if(sel){float pu=.5f+.5f*sinf(sceneTime*3);DrawRectangleLinesEx(r,3+pu*2,kAccent);}
        DrawText(tile->getCode().c_str(),int(r.x+5),int(r.y+5),16,kText);
        DrawText(tile->getName().c_str(),int(r.x+5),int(r.y+25),10,kSubtext);

        PropertyTile* pt=dynamic_cast<PropertyTile*>(tile);
        if(pt&&pt->getOwner()){
            int oi=0; for(auto& p:g->getPlayers()){if(&p==pt->getOwner())break;oi++;}
            DrawCircle(int(r.x+r.width-14),int(r.y+14),7,kTokens[oi%4]);}
        if(pt&&pt->isMortgaged()) DrawText("G",int(r.x+r.width-22),int(r.y+26),14,kDanger);
        if(st){for(int h=0;h<st->getHouseCount()&&h<4;++h) DrawRectangle(int(r.x+7+h*11),int(r.y+r.height-34),8,8,kAccentAlt);
            if(st->hasHotelBuilt()) DrawRectangle(int(r.x+7),int(r.y+r.height-34),16,10,kDanger);}
        if(pt&&pt->getFestivalMultiplier()>1){std::string fs="x"+std::to_string(pt->getFestivalMultiplier());
            DrawText(fs.c_str(),int(r.x+r.width-26),int(r.y+r.height-22),13,{180,50,200,255});}
    }

    drawCenterPanel(br);

    // Tokens
    for(size_t i=0;i<tokenPos.size();++i){
        float bob=sinf(tokenPhase[i]*2)*4;
        Vector2 pos{tokenPos[i].x,tokenPos[i].y+bob};
        Color tint=kTokens[i%4];
        DrawCircleLines(int(pos.x),int(pos.y),14+sinf(tokenPhase[i]*2.5f)*2,Fade(tint,.3f));
        DrawCircle(int(pos.x),int(pos.y),10,tint);
        DrawCircleLines(int(pos.x),int(pos.y),10,Fade(WHITE,.5f));
        DrawText(std::to_string(int(i)+1).c_str(),int(pos.x-4),int(pos.y-6),12,kText);
    }
}

void InGameScene::drawSidebar(const Rectangle& sb) {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || g->getPlayers().empty()) return;

    DrawRectangleRounded({sb.x + 4, sb.y + 7, sb.width, sb.height}, .04f, 10, Fade(kText, .07f));
    DrawRectangleRounded(sb, .04f, 10, Fade(kPanel, .97f));
    DrawRectangleRoundedLinesEx(sb, .04f, 10, 2, Fade(kPanelBorder, .8f));
    drawSmallFlower(sb.x + sb.width - 22, sb.y + 22, 9, sceneTime * .4f, .35f);

    int ci = g->getTurnManager().getCurrentPlayerIndex();
    if (ci < 0 || ci >= static_cast<int>(g->getPlayers().size())) return;

    Player& cur = g->getPlayer(ci);

    DrawText("Giliran Sekarang", int(sb.x + 16), int(sb.y + 16), 24, kText);
    DrawCircle(int(sb.x + 28), int(sb.y + 66), 11, kTokens[ci % 4]);
    DrawText(cur.getUsername().c_str(), int(sb.x + 50), int(sb.y + 52), 26, kText);
    DrawText(("M" + std::to_string(cur.getMoney())).c_str(), int(sb.x + 16), int(sb.y + 88), 22, kAccentAlt);

    if (g->getBoard().size() > 0 && selectedTile >= 0 && selectedTile < g->getBoard().size()) {
        Tile* selected = g->getBoard().getTileByIndex(selectedTile);

        if (selected != nullptr) {
            DrawText("Petak Dipilih", int(sb.x + 16), int(sb.y + 116), 18, kText);
            DrawText(
                (selected->getCode() + " - " + selected->getName()).c_str(),
                int(sb.x + 16),
                int(sb.y + 140),
                17,
                kSubtext
            );

            PropertyTile* pt = dynamic_cast<PropertyTile*>(selected);
            if (pt != nullptr) {
                std::string status =
                    pt->isMortgaged() ? "GADAI" :
                    pt->isOwned() ? "OWNED" :
                    "BANK";

                DrawText(
                    ("Status: " + status).c_str(),
                    int(sb.x + 16),
                    int(sb.y + 162),
                    16,
                    kSubtext
                );
            }
        }
    }

    Rectangle wave{sb.x + 16, sb.y + 190, sb.width - 32, 16};
    DrawRectangleRounded(wave, .8f, 10, Fade(kPanelBorder, .35f));

    float prog = g->getTurnManager().getMaxTurn() > 0
        ? static_cast<float>(g->getTurnManager().getCurrentTurn()) / g->getTurnManager().getMaxTurn()
        : .5f;

    DrawRectangleRounded({wave.x, wave.y, wave.width * prog, wave.height}, .8f, 10, kAccent);

    if (!actionButtons.empty()) {
        actionButtons[0].disabled = g->isGameOver() || g->getHasRolledThisTurn();
    }

    for (Button& b : actionButtons) {
        b.draw();
    }

    DrawText("Semua Pemain", int(sb.x + 16), int(sb.y + 640), 22, kText);

    for (size_t i = 0; i < g->getPlayers().size(); ++i) {
        float py = sb.y + 672 + i * 28;
        DrawCircle(int(sb.x + 20), int(py + 8), 6, kTokens[i % 4]);

        Player& p = g->getPlayers()[i];

        std::string row = p.getUsername() + " M" + std::to_string(p.getMoney());

        if (p.isBankrupt()) {
            row += " [BANGKRUT]";
        } else if (p.getStatus() == PlayerStatus::JAILED) {
            row += " [PENJARA]";
        }

        DrawText(row.c_str(), int(sb.x + 36), int(py), 17, int(i) == ci ? kText : kSubtext);
    }

    DrawText("Log Terbaru", int(sb.x + 16), int(sb.y + 790), 22, kText);

    auto entries = g->getLogger().getEntries();
    int logCount = std::min<int>(4, static_cast<int>(entries.size()));

    for (int i = 0; i < logCount; ++i) {
        const LogEntry& e = entries[entries.size() - 1 - static_cast<size_t>(i)];
        float ly = sb.y + 822 + i * 40;

        DrawText(
            ("[T" + std::to_string(e.getTurn()) + "] " + e.getUsername() + " | " + e.getActionType()).c_str(),
            int(sb.x + 16),
            int(ly),
            16,
            kText
        );

        DrawText(
            e.getDetail().c_str(),
            int(sb.x + 16),
            int(ly + 17),
            15,
            kSubtext
        );
    }
}

void InGameScene::drawOverlay(Rectangle sr) {
    if (overlayVis <= .01f || !overlayOpen) return;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(kText, .38f * overlayVis));

    float rise = (1 - overlayVis) * 24;
    Rectangle p{
        sr.width * .5f - 310,
        sr.height * .5f - 252 + rise,
        620,
        504
    };

    DrawRectangleRounded({p.x + 5, p.y + 9, p.width, p.height}, .09f, 10, Fade(kText, .12f * overlayVis));
    DrawRectangleRounded(p, .09f, 10, Fade({250,255,235,255}, overlayVis));
    DrawRectangleRoundedLinesEx(p, .09f, 10, 2.5f, Fade(kPanelBorder, overlayVis));

    drawSmallFlower(p.x + p.width - 28, p.y + 28, 14, sceneTime * .5f, .5f * overlayVis);

    DrawText(overlayTitle.c_str(), int(p.x + 22), int(p.y + 20), 32, kText);

    closeOverlayBtn.draw();

    float y = p.y + 80;

    for (const auto& line : overlayLines) {
        DrawText(line.c_str(), int(p.x + 24), int(y), 20, kSubtext);
        y += 32;

        if (y > p.y + p.height - 70) break;
    }

    if (!overlayFooter.empty()) {
        DrawText(
            overlayFooter.c_str(),
            int(p.x + 24),
            int(p.y + p.height - 42),
            18,
            kSubtext
        );
    }
}

void InGameScene::drawSaveModal(Rectangle sr) {
    if (saveModalVis <= .01f || !showSaveModal) return;

    DrawRectangle(
        0,
        0,
        GetScreenWidth(),
        GetScreenHeight(),
        Fade(kText, .38f * saveModalVis)
    );

    Rectangle p{
        sr.width * .5f - 300,
        sr.height * .5f - 170 + (1 - saveModalVis) * 24,
        600,
        340
    };

    DrawRectangleRounded(
        {p.x + 5, p.y + 9, p.width, p.height},
        .09f,
        10,
        Fade(kText, .12f * saveModalVis)
    );

    DrawRectangleRounded(
        p,
        .09f,
        10,
        Fade({250,255,235,255}, saveModalVis)
    );

    DrawRectangleRoundedLinesEx(
        p,
        .09f,
        10,
        2.5f,
        Fade(kPanelBorder, saveModalVis)
    );

    DrawText(
        "Simpan Game",
        static_cast<int>(p.x + 24),
        static_cast<int>(p.y + 24),
        32,
        kText
    );

    DrawText(
        "Path Save File:",
        static_cast<int>(p.x + 24),
        static_cast<int>(p.y + 92),
        20,
        kText
    );

    savePathField.setBoundary({
        p.x + 24,
        p.y + 122,
        p.width - 48,
        44
    });

    savePathField.draw();

    if (!saveError.empty()) {
        DrawText(
            saveError.c_str(),
            static_cast<int>(p.x + 24),
            static_cast<int>(p.y + 184),
            18,
            kDanger
        );
    }

    saveConfirmButton.setBoundary({
        p.x + p.width - 220,
        p.y + p.height - 70,
        156,
        50
    });

    saveCancelButton.setBoundary({
        p.x + p.width - 390,
        p.y + p.height - 70,
        140,
        50
    });

    saveConfirmButton.draw();
    saveCancelButton.draw();
}

void InGameScene::drawDiceModal(Rectangle sr) {
    if (diceModalVis <= .01f || !showDiceModal) return;

    DrawRectangle(
        0,
        0,
        GetScreenWidth(),
        GetScreenHeight(),
        Fade(kText, .38f * diceModalVis)
    );

    Rectangle p{
        sr.width * .5f - 280,
        sr.height * .5f - 170 + (1 - diceModalVis) * 24,
        560,
        340
    };

    DrawRectangleRounded(
        {p.x + 5, p.y + 9, p.width, p.height},
        .09f,
        10,
        Fade(kText, .12f * diceModalVis)
    );

    DrawRectangleRounded(
        p,
        .09f,
        10,
        Fade({250,255,235,255}, diceModalVis)
    );

    DrawRectangleRoundedLinesEx(
        p,
        .09f,
        10,
        2.5f,
        Fade(kPanelBorder, diceModalVis)
    );

    drawSmallFlower(p.x + p.width - 28, p.y + 28, 14, sceneTime * .5f, .5f * diceModalVis);

    DrawText(
        diceManualMode ? "Atur Dadu" : "Dadu",
        static_cast<int>(p.x + 24),
        static_cast<int>(p.y + 24),
        32,
        kText
    );

    if (diceManualMode) {
        DrawText(
            "Masukkan nilai masing-masing dadu.",
            static_cast<int>(p.x + 24),
            static_cast<int>(p.y + 78),
            20,
            kSubtext
        );

        DrawText("Dadu 1", static_cast<int>(p.x + 80), static_cast<int>(p.y + 106), 18, kText);
        DrawText("Dadu 2", static_cast<int>(p.x + p.width - 230), static_cast<int>(p.y + 106), 18, kText);

        diceOneField.setBoundary({
            p.x + 80,
            p.y + 132,
            150,
            52
        });

        diceTwoField.setBoundary({
            p.x + p.width - 230,
            p.y + 132,
            150,
            52
        });

        diceOneField.draw();
        diceTwoField.draw();

        if (!diceError.empty()) {
            DrawText(
                diceError.c_str(),
                static_cast<int>(p.x + 24),
                static_cast<int>(p.y + 208),
                18,
                kDanger
            );
        }

        diceManualConfirmButton.setBoundary({
            p.x + p.width - 204,
            p.y + p.height - 72,
            156,
            50
        });

        diceBackButton.setBoundary({
            p.x + 48,
            p.y + p.height - 72,
            140,
            50
        });

        diceManualConfirmButton.draw();
        diceBackButton.draw();
    } else {
        DrawText(
            "Pilih cara menentukan hasil dadu.",
            static_cast<int>(p.x + 24),
            static_cast<int>(p.y + 78),
            20,
            kSubtext
        );

        diceRollButton.setBoundary({
            p.x + 48,
            p.y + 128,
            p.width - 96,
            52
        });

        diceManualButton.setBoundary({
            p.x + 48,
            p.y + 196,
            p.width - 96,
            52
        });

        diceCancelButton.setBoundary({
            p.x + p.width - 188,
            p.y + p.height - 72,
            140,
            50
        });

        diceRollButton.draw();
        diceManualButton.draw();
        diceCancelButton.draw();
    }
}

void InGameScene::draw() {
    Rectangle sr{
        0,
        0,
        static_cast<float>(GetScreenWidth()),
        static_cast<float>(GetScreenHeight())
    };

    Rectangle br{};
    Rectangle sb{};

    layoutUi(sr, br, sb);

    drawBackground(sr);
    drawHeader(sr);
    drawBoard(br);
    drawSidebar(sb);
    drawOverlay(sr);
    drawSaveModal(sr);
    drawDiceModal(sr);
}
