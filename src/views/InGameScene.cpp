#include "../../include/views/InGameScene.hpp"
#include "../../include/views/SceneManager.hpp"
#include "../../include/core/GameManager.hpp"
#include "../../include/core/Game.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/models/TradeToPlayer.hpp"
#include "../../include/core/TradeManager.hpp"
#include "../../include/utils/Board.hpp"
#include "../../include/utils/Tile.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/utils/RailroadTile.hpp"
#include "../../include/utils/UtilityTile.hpp"
#include "../../include/utils/LogEntry.hpp"
#include "../../include/core/TurnManager.hpp"
#include "../../include/core/HandOverflowException.hpp"
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
#include <sstream>

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

    std::string fitText(const std::string& value, int fontSize, int maxWidth) {
        if (MeasureText(value.c_str(), fontSize) <= maxWidth) {
            return value;
        }

        const std::string suffix = "...";
        std::string result = value;

        while (!result.empty() && MeasureText((result + suffix).c_str(), fontSize) > maxWidth) {
            result.pop_back();
        }

        return result.empty() ? suffix : result + suffix;
    }

    std::string formatMoney(int amount) {
        std::string digits = std::to_string(amount);
        std::string formatted;
        int count = 0;

        for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
            if (count == 3) {
                formatted.push_back('.');
                count = 0;
            }

            formatted.push_back(*it);
            ++count;
        }

        std::reverse(formatted.begin(), formatted.end());
        return "M" + formatted;
    }

    std::string upperCopy(std::string value) {
        for (char& c : value) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }

        return value;
    }

    bool parseMoneyField(const std::string& raw, int& amount, std::string& error, const std::string& label) {
        std::string value = trimCopy(raw);

        if (value.empty()) {
            amount = 0;
            return true;
        }

        for (char c : value) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                error = label + " harus berupa angka 0 atau lebih.";
                return false;
            }
        }

        try {
            long long parsed = std::stoll(value);
            if (parsed > 2147483647LL) {
                error = label + " terlalu besar.";
                return false;
            }

            amount = static_cast<int>(parsed);
            return true;
        } catch (const std::exception&) {
            error = label + " tidak valid.";
            return false;
        }
    }

    Player* findPlayerByUsername(Game* game, const std::string& rawName) {
        if (game == nullptr) {
            return nullptr;
        }

        const std::string targetName = trimCopy(rawName);

        for (Player& player : game->getPlayers()) {
            if (player.getUsername() == targetName) {
                return &player;
            }
        }

        return nullptr;
    }

    PropertyTile* findPropertyByCode(Game* game, const std::string& code) {
        if (game == nullptr) {
            return nullptr;
        }

        const std::string wanted = upperCopy(trimCopy(code));
        const auto& tiles = game->getBoard().getTiles();

        for (Tile* tile : tiles) {
            PropertyTile* property = dynamic_cast<PropertyTile*>(tile);
            if (property != nullptr && upperCopy(property->getCode()) == wanted) {
                return property;
            }
        }

        return nullptr;
    }

    bool parsePropertyList(Game* game, const std::string& raw, std::vector<PropertyTile*>& props, std::string& error, const std::string& label) {
        props.clear();

        std::string normalized = raw;
        for (char& c : normalized) {
            if (c == ',' || c == ';') {
                c = ' ';
            }
        }

        std::stringstream ss(normalized);
        std::string token;
        std::vector<std::string> seenCodes;

        while (ss >> token) {
            const std::string code = upperCopy(trimCopy(token));
            if (code.empty()) {
                continue;
            }

            if (std::find(seenCodes.begin(), seenCodes.end(), code) != seenCodes.end()) {
                error = "Properti " + code + " muncul lebih dari sekali di " + label + ".";
                return false;
            }

            PropertyTile* property = findPropertyByCode(game, code);
            if (property == nullptr) {
                error = "Properti " + code + " pada " + label + " tidak valid.";
                return false;
            }

            seenCodes.push_back(code);
            props.push_back(property);
        }

        return true;
    }

    std::string propertyCodesLine(const std::vector<PropertyTile*>& props) {
        if (props.empty()) {
            return "-";
        }

        std::string result;
        for (std::size_t i = 0; i < props.size(); ++i) {
            if (i > 0) {
                result += ", ";
            }

            result += props[i]->getCode();
        }

        return result;
    }

    std::vector<std::string> buildTradeLines(TradeToPlayer* trade) {
        if (trade == nullptr) {
            return {"Penawaran trade tidak tersedia."};
        }

        Player* proposer = trade->getProposer();
        Player* target = trade->getTarget();

        return {
            "Dari: " + std::string(proposer != nullptr ? proposer->getUsername() : "-"),
            "Untuk: " + std::string(target != nullptr ? target->getUsername() : "-"),
            "Diberikan: " + propertyCodesLine(trade->getOfferedProperties()) + " + " + formatMoney(trade->getOfferedMoney()),
            "Diminta: " + propertyCodesLine(trade->getRequestedProperties()) + " + " + formatMoney(trade->getRequestedMoney())
        };
    }

    std::vector<std::string> buildRankingLines(Game* game) {
        std::vector<std::string> lines;

        if (game == nullptr) {
            return lines;
        }

        std::vector<Player*> sorted;

        for (Player& player : game->getPlayers()) {
            sorted.push_back(&player);
        }

        std::sort(sorted.begin(), sorted.end(), [](Player* a, Player* b) {
            return a->getTotalWealth() > b->getTotalWealth();
        });

        lines.push_back("=== Klasemen Akhir ===");

        for (std::size_t i = 0; i < sorted.size(); ++i) {
            std::string row =
                std::to_string(i + 1) + ". " +
                sorted[i]->getUsername() +
                " - " + formatMoney(sorted[i]->getTotalWealth());

            if (sorted[i]->isBankrupt()) {
                row += " [BANGKRUT]";
            }

            lines.push_back(row);
        }

        return lines;
    }

    std::string displayName(std::string value) {
        for (char& c : value) {
            if (c == '_') {
                c = ' ';
            }
        }

        return value;
    }

    std::string propertyStatusLine(const PropertyTile* property) {
        if (property == nullptr) {
            return "Status : -";
        }

        std::string status = "BANK";
        if (property->isMortgaged()) {
            status = "MORTGAGED";
        } else if (property->isOwned()) {
            status = "OWNED";
        }

        if (property->getOwner() != nullptr) {
            status += " (" + property->getOwner()->getUsername() + ")";
        }

        return "Status : " + status;
    }

    std::vector<std::string> buildPropertyInfoLines(Game* game, PropertyTile* property) {
        if (property == nullptr) {
            return {};
        }

        std::vector<std::string> lines;
        const std::string separator = "------------------------------";

        StreetTile* street = dynamic_cast<StreetTile*>(property);
        RailroadTile* railroad = dynamic_cast<RailroadTile*>(property);
        UtilityTile* utility = dynamic_cast<UtilityTile*>(property);

        if (street != nullptr) {
            const std::vector<int>& rents = street->getRentTable();
            const std::string color = displayName(street->getColorGroup());

            lines.push_back("[" + color + "] " + displayName(street->getName()) + " (" + street->getCode() + ")");
            lines.push_back(separator);
            lines.push_back("Harga Beli : " + formatMoney(street->getLandPrice()));
            lines.push_back("Nilai Gadai : " + formatMoney(street->getMortgageValue()));
            lines.push_back(separator);

            if (rents.size() >= 6U) {
                lines.push_back("Sewa (unimproved) : " + formatMoney(rents[0]));
                lines.push_back("Sewa (1 rumah) : " + formatMoney(rents[1]));
                lines.push_back("Sewa (2 rumah) : " + formatMoney(rents[2]));
                lines.push_back("Sewa (3 rumah) : " + formatMoney(rents[3]));
                lines.push_back("Sewa (4 rumah) : " + formatMoney(rents[4]));
                lines.push_back("Sewa (hotel) : " + formatMoney(rents[5]));
            } else {
                lines.push_back("Sewa : data tidak tersedia");
            }

            lines.push_back(separator);
            lines.push_back("Harga Rumah : " + formatMoney(street->getHouseBuildCost()));
            lines.push_back("Harga Hotel : " + formatMoney(street->getHotelBuildCost()));
            lines.push_back(separator);
            lines.push_back("Jumlah Rumah : " + std::to_string(street->getHouseCount()));
            lines.push_back("Jumlah Hotel : " + std::to_string(street->hasHotelBuilt() ? 1 : 0));
        } else if (railroad != nullptr) {
            lines.push_back("[RAILROAD] " + displayName(railroad->getName()) + " (" + railroad->getCode() + ")");
            lines.push_back(separator);
            lines.push_back("Harga Beli : " + formatMoney(railroad->getLandPrice()));
            lines.push_back("Nilai Gadai : " + formatMoney(railroad->getMortgageValue()));
            lines.push_back(separator);

            for (const auto& entry : railroad->getRentTable()) {
                lines.push_back(
                    "Sewa (" + std::to_string(entry.first) + " railroad) : " +
                    formatMoney(entry.second)
                );
            }
        } else if (utility != nullptr) {
            lines.push_back("[UTILITY] " + displayName(utility->getName()) + " (" + utility->getCode() + ")");
            lines.push_back(separator);
            lines.push_back("Harga Beli : " + formatMoney(utility->getLandPrice()));
            lines.push_back("Nilai Gadai : " + formatMoney(utility->getMortgageValue()));
            lines.push_back(separator);

            for (const auto& entry : utility->getMultiplierTable()) {
                lines.push_back(
                    "Faktor Sewa (" + std::to_string(entry.first) + " utility) : x" +
                    std::to_string(entry.second) + " dadu"
                );
            }
        } else {
            lines.push_back(displayName(property->getName()) + " (" + property->getCode() + ")");
            lines.push_back("Harga Beli : " + formatMoney(property->getLandPrice()));
            lines.push_back("Nilai Gadai : " + formatMoney(property->getMortgageValue()));
        }

        lines.push_back(separator);
        lines.push_back(propertyStatusLine(property));

        if (property->getFestivalMultiplier() > 1) {
            lines.push_back(
                "Festival : x" + std::to_string(property->getFestivalMultiplier()) +
                " (" + std::to_string(property->getFestivalDuration()) + " giliran)"
            );
            lines.push_back("Sewa terbaru : " + formatMoney(property->calculateRent(nullptr, game)));
        }

        return lines;
    }

    std::vector<const StreetTile*> collectStreetGroup(Game* game, const std::string& colorGroup) {
        std::vector<const StreetTile*> group;
        if (game == nullptr) {
            return group;
        }

        for (Tile* tile : game->getBoard().getTiles()) {
            const StreetTile* street = dynamic_cast<const StreetTile*>(tile);
            if (street != nullptr && street->getColorGroup() == colorGroup) {
                group.push_back(street);
            }
        }

        return group;
    }

    std::string buildingStatusText(const StreetTile* street) {
        if (street == nullptr) {
            return "-";
        }

        if (street->hasHotelBuilt()) {
            return "Hotel";
        }

        return std::to_string(street->getHouseCount()) + " rumah";
    }

    std::vector<std::string> buildGroupStatusLines(Game* game, const std::string& colorGroup) {
        std::vector<std::string> lines;
        const std::vector<const StreetTile*> group = collectStreetGroup(game, colorGroup);

        lines.push_back("Color group [" + displayName(colorGroup) + "]:");
        for (const StreetTile* street : group) {
            lines.push_back(
                "- " + displayName(street->getName()) + " (" + street->getCode() + ") : " +
                buildingStatusText(street)
            );
        }

        return lines;
    }

    std::vector<std::string> buildEligibleBuildLines(Game* game) {
        std::vector<std::string> lines;
        if (game == nullptr || game->getBoard().size() == 0) {
            return lines;
        }

        const int playerIndex = game->getTurnManager().getCurrentPlayerIndex();
        if (playerIndex < 0 || playerIndex >= static_cast<int>(game->getPlayers().size())) {
            return lines;
        }

        Player& current = game->getPlayer(playerIndex);
        std::vector<std::string> visitedGroups;
        int groupNumber = 1;

        lines.push_back("=== Color Group yang Memenuhi Syarat ===");

        for (Tile* tile : game->getBoard().getTiles()) {
            const StreetTile* street = dynamic_cast<const StreetTile*>(tile);
            if (street == nullptr) {
                continue;
            }

            const std::string colorGroup = street->getColorGroup();
            if (std::find(visitedGroups.begin(), visitedGroups.end(), colorGroup) != visitedGroups.end()) {
                continue;
            }
            visitedGroups.push_back(colorGroup);

            const std::vector<const StreetTile*> group = collectStreetGroup(game, colorGroup);
            std::vector<std::string> buildableLines;

            for (const StreetTile* candidate : group) {
                if (candidate->getOwner() != &current) {
                    continue;
                }

                if (candidate->canBuildHouse(group)) {
                    buildableLines.push_back(
                        "- " + displayName(candidate->getName()) + " (" + candidate->getCode() + ") : " +
                        buildingStatusText(candidate) + " (Harga rumah: " +
                        formatMoney(candidate->getHouseBuildCost()) + ")"
                    );
                } else if (candidate->canBuildHotel(group)) {
                    buildableLines.push_back(
                        "- " + displayName(candidate->getName()) + " (" + candidate->getCode() + ") : " +
                        buildingStatusText(candidate) + " (Upgrade hotel: " +
                        formatMoney(candidate->getHotelBuildCost()) + ")"
                    );
                }
            }

            if (buildableLines.empty()) {
                continue;
            }

            lines.push_back(std::to_string(groupNumber) + ". [" + displayName(colorGroup) + "]");
            lines.insert(lines.end(), buildableLines.begin(), buildableLines.end());
            ++groupNumber;
        }

        if (groupNumber == 1) {
            lines.clear();
            lines.push_back("Tidak ada color group yang memenuhi syarat untuk dibangun.");
            lines.push_back("Kamu harus memiliki seluruh petak dalam satu color group.");
            return lines;
        }

        lines.push_back("Uang kamu saat ini : " + formatMoney(current.getMoney()));
        lines.push_back("Pilih petak yang tampil di daftar, lalu tekan Bangun.");
        return lines;
    }

    std::vector<std::string> buildAuctionResultLines(AuctionManager& auction) {
        PropertyTile* property = auction.getLastProperty();
        Player* winner = auction.getLastWinner();

        std::vector<std::string> lines;
        if (property != nullptr) {
            lines.push_back(
                "Properti: " + displayName(property->getName()) + " (" + property->getCode() + ")"
            );
        }

        lines.push_back("Lelang selesai!");

        if (auction.getLastAuctionHadWinner() && winner != nullptr) {
            lines.push_back("Pemenang: " + winner->getUsername());
            lines.push_back("Harga akhir: " + formatMoney(auction.getLastWinningBid()));
            if (property != nullptr) {
                lines.push_back(
                    "Properti " + displayName(property->getName()) + " kini dimiliki " +
                    winner->getUsername() + "."
                );
            }
        } else {
            lines.push_back("Tidak ada pemenang.");
            lines.push_back("Properti tetap dimiliki Bank.");
        }

        return lines;
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
      openLogButton("Buka Log", kAccentAlt, {255,255,255,255}),
      closeLogButton("X", kDanger, {255,255,255,255}),
      closeCardButton("Tutup", kPanelBorder, kText),
      useCardButton("Pakai", kAccentAlt, {255,255,255,255}),
      discardCardButton("Buang", kDanger, {255,255,255,255}),
      cancelLassoTargetButton("Batal", kPanelBorder, kText),
      auctionBidButton("Bid", kAccentAlt, {255,255,255,255}),
      auctionPassButton("Pass", kDanger, {255,255,255,255}),
      auctionCloseButton("Tutup", kAccentAlt, {255,255,255,255}),
      tradeSendButton("Kirim", kAccentAlt, {255,255,255,255}),
      tradeCancelButton("Batal", kPanelBorder, kText),
      tradeAcceptButton("Terima", kAccentAlt, {255,255,255,255}),
      tradeRejectButton("Tolak", kDanger, {255,255,255,255}),
      savePathField("data/save.txt"),
      diceOneField("1-6"),
      diceTwoField("1-6"),
      auctionBidField("Jumlah bid"),
      tradeTargetField("Nama player"),
      tradeOfferPropsField("Kode properti diberikan"),
      tradeOfferMoneyField("Uang diberikan"),
      tradeRequestPropsField("Kode properti diminta"),
      tradeRequestMoneyField("Uang diminta"),
      sceneTime(0),
      selectedTile(0),
      overlayOpen(false),
      overlayVis(0),
      showSaveModal(false),
      saveModalVis(0),
      showDiceModal(false),
      diceManualMode(false),
      diceModalVis(0),
      showLogModal(false),
      logModalVis(0),
      showCardModal(false),
      cardOverflowMode(false),
      selectedCardIndex(-1),
      cardModalVis(0),
      showLassoTargetModal(false),
      pendingLassoCardIndex(-1),
      lassoTargetModalVis(0),
      propertyDecisionPending(false),
      propertyDecisionResolved(false),
      pendingProperty(nullptr),
      showAuctionModal(false),
      auctionNoticeMode(false),
      auctionModalVis(0),
      showTradeModal(false),
      tradeResponseMode(false),
      pendingTrade(nullptr),
      tradeModalVis(0) {

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

            refreshPropertyDecisionState();
            if (hasBlockingPropertyDecision()) {
                showOverlay(
                    "Pilih Aksi Properti",
                    {
                        "Properti " + displayName(pendingProperty->getName()) + " (" + pendingProperty->getCode() + ") belum diproses.",
                        "Pilih Beli atau Lelang sebelum melempar dadu."
                    }
                );
                return;
            }

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

        {"Bayar Penjara", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr || g->isGameOver()) return;

            int idx = g->getTurnManager().getCurrentPlayerIndex();
            if (idx < 0 || idx >= static_cast<int>(g->getPlayers().size())) return;
            Player& current = g->getPlayer(idx);
            int fine = g->getConfig().getSpecialConfig(JAIL_FINE);

            bool ok = g->payJailFineForCurrentPlayer();
            showOverlay(
                ok ? "Keluar Penjara" : "Bayar Penjara Gagal",
                {
                    ok
                        ? current.getUsername() + " keluar penjara dengan membayar " + formatMoney(fine) + "."
                        : "Pembayaran hanya bisa dilakukan saat pemain aktif berada di penjara dan belum melempar dadu."
                }
            );
        }},

        {"Kartu Penjara", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr || g->isGameOver()) return;

            bool ok = g->useJailFreeCardForCurrentPlayer();
            showOverlay(
                ok ? "Keluar Penjara" : "Kartu Tidak Tersedia",
                {
                    ok
                        ? "Kartu Bebas dari Penjara digunakan."
                        : "Pemain aktif harus berada di penjara, belum melempar dadu, dan memiliki kartu Bebas dari Penjara."
                }
            );
        }},

        {"Beli", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr || g->isGameOver()) return;

            refreshPropertyDecisionState();
            if (!propertyDecisionPending || pendingProperty == nullptr) {
                showOverlay("Pembelian Gagal", {"Tidak ada properti bank yang wajib dipilih saat ini."});
                return;
            }

            int idx = g->getTurnManager().getCurrentPlayerIndex();
            if (idx < 0 || idx >= static_cast<int>(g->getPlayers().size())) return;
            Player& current = g->getPlayer(idx);
            int amount = pendingProperty->getLandPrice();
            if (current.getDiscountDuration() > 0) {
                amount = amount * (100 - current.getDiscountPercent()) / 100;
            }

            if (!current.canAfford(amount)) {
                startAuctionForProperty(pendingProperty, "Uang pemain tidak cukup untuk membeli properti.");
                return;
            }

            bool ok = g->buyCurrentProperty();
            if (ok) {
                propertyDecisionPending = false;
                propertyDecisionResolved = true;
                pendingProperty = nullptr;
            }

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
                showOverlay("Bangun", buildEligibleBuildLines(g));
                return;
            }

            Tile* selected = g->getBoard().getTileByIndex(selectedTile);
            StreetTile* street = dynamic_cast<StreetTile*>(selected);
            if (street == nullptr) {
                std::vector<std::string> lines = {
                    "Pilih petak street/property yang memenuhi syarat untuk dibangun."
                };
                const std::vector<std::string> options = buildEligibleBuildLines(g);
                lines.insert(lines.end(), options.begin(), options.end());
                showOverlay("Bangun Gagal", lines);
                return;
            }

            const std::vector<const StreetTile*> group = collectStreetGroup(g, street->getColorGroup());
            const bool willBuildHouse = street->canBuildHouse(group);
            const bool willBuildHotel = street->canBuildHotel(group);
            if (!willBuildHouse && !willBuildHotel) {
                std::vector<std::string> lines = {
                    displayName(street->getName()) + " belum memenuhi syarat bangun.",
                    "Rumah hanya bisa dibangun jika seluruh color group dimiliki pemain aktif dan jumlah rumah tetap merata.",
                    "Hotel hanya bisa dibangun jika semua petak dalam color group sudah mencapai 4 rumah atau hotel."
                };
                const std::vector<std::string> options = buildEligibleBuildLines(g);
                lines.insert(lines.end(), options.begin(), options.end());
                showOverlay("Bangun Gagal", lines);
                return;
            }

            int playerIndex = g->getTurnManager().getCurrentPlayerIndex();
            if (playerIndex < 0 || playerIndex >= static_cast<int>(g->getPlayers().size())) return;
            Player& current = g->getPlayer(playerIndex);
            const int cost = willBuildHotel ? street->getHotelBuildCost() : street->getHouseBuildCost();
            if (!current.canAfford(cost)) {
                showOverlay(
                    "Bangun Gagal",
                    {
                        "Uang tidak cukup untuk membangun di " + displayName(street->getName()) + ".",
                        "Biaya: " + formatMoney(cost),
                        "Uang kamu saat ini: " + formatMoney(current.getMoney())
                    }
                );
                return;
            }

            const std::string builtName = displayName(street->getName());
            const std::string colorGroup = street->getColorGroup();
            bool ok = g->buildProperty(code);

            std::vector<std::string> resultLines;
            if (ok) {
                resultLines.push_back(
                    willBuildHotel
                        ? builtName + " di-upgrade ke Hotel!"
                        : "Kamu membangun 1 rumah di " + builtName + ". Biaya: " + formatMoney(cost)
                );
                resultLines.push_back("Uang tersisa: " + formatMoney(current.getMoney()));
                const std::vector<std::string> statusLines = buildGroupStatusLines(g, colorGroup);
                resultLines.insert(resultLines.end(), statusLines.begin(), statusLines.end());
            } else {
                resultLines.push_back("Pilih street milik pemain aktif dan pastikan syarat bangun terpenuhi.");
            }

            showOverlay(
                ok ? "Bangun Berhasil" : "Bangun Gagal",
                resultLines
            );
        }},

        {"Info Petak", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr || g->getBoard().size() == 0) return;

            Tile* t = g->getBoard().getTileByIndex(selectedTile);
            if (t == nullptr) return;

            PropertyTile* pt = dynamic_cast<PropertyTile*>(t);
            if (pt != nullptr) {
                showOverlay("AKTA KEPEMILIKAN", buildPropertyInfoLines(g, pt), "Klik X untuk menutup.");
                return;
            }

            showOverlay(
                displayName(t->getName()),
                {
                    "Kode : " + t->getCode(),
                    "Nama : " + displayName(t->getName())
                },
                "Klik X untuk menutup."
            );
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
                if (pt->getFestivalMultiplier() > 1) {
                    row += " [Festival x" + std::to_string(pt->getFestivalMultiplier()) +
                           ", " + std::to_string(pt->getFestivalDuration()) +
                           " giliran, sewa " + formatMoney(pt->calculateRent(nullptr, g)) + "]";
                }
                lines.push_back(row);
            }

            if (lines.size() == 1) {
                lines.push_back("(tidak ada properti)");
            }

            showOverlay("Properti " + p.getUsername(), lines);
        }},

        {"Kartu", [this]() {
            openCardModal(false);
        }},

        {"Festival", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr) return;

            if (g->isFestivalSelectionPendingForCurrentPlayer()) {
                int idx = g->getTurnManager().getCurrentPlayerIndex();
                if (idx < 0 || idx >= static_cast<int>(g->getPlayers().size())) return;

                Tile* selected = nullptr;
                if (g->getBoard().size() > 0 && selectedTile >= 0 && selectedTile < g->getBoard().size()) {
                    selected = g->getBoard().getTileByIndex(selectedTile);
                }

                PropertyTile* property = dynamic_cast<PropertyTile*>(selected);
                if (property == nullptr) {
                    std::vector<std::string> lines = g->buildFestivalSelectionLines(g->getPlayer(idx));
                    lines.push_back("Pilih petak properti milikmu, lalu klik Festival lagi.");
                    showOverlay("Festival", lines);
                    return;
                }

                std::vector<std::string> lines;
                bool ok = g->applyFestivalToCurrentPlayerProperty(property->getCode(), &lines);
                if (!ok) {
                    const std::vector<std::string> options = g->buildFestivalSelectionLines(g->getPlayer(idx));
                    lines.insert(lines.end(), options.begin(), options.end());
                    lines.push_back("Pilih petak properti milikmu, lalu klik Festival lagi.");
                }

                showOverlay(ok ? "Festival Aktif" : "Festival", lines);
                return;
            }

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

        {"Trade", [this]() {
            openTradeProposal();
        }},

        {"Lelang", [this]() {
            Game* g = gameManager->getCurrentGame();
            if (g == nullptr) return;

            refreshPropertyDecisionState();
            if (propertyDecisionPending && pendingProperty != nullptr) {
                startAuctionForProperty(pendingProperty, "Pemain memilih melelang properti.");
                return;
            }

            if (g->getAuctionManager().isAuctionActive()) {
                showAuctionModal = true;
                auctionNoticeMode = false;
                overlayOpen = false;
                return;
            }

            showOverlay("Lelang", {"Tidak ada properti bank yang sedang menunggu keputusan lelang."});
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

            refreshPropertyDecisionState();
            if (g->getAuctionManager().isAuctionActive() || showAuctionModal) {
                showAuctionModal = true;
                overlayOpen = false;
                return;
            }

            if (g->isFestivalSelectionPendingForCurrentPlayer()) {
                int idx = g->getTurnManager().getCurrentPlayerIndex();
                std::vector<std::string> lines = {"Selesaikan pilihan Festival terlebih dahulu."};
                if (idx >= 0 && idx < static_cast<int>(g->getPlayers().size())) {
                    const std::vector<std::string> options = g->buildFestivalSelectionLines(g->getPlayer(idx));
                    lines.insert(lines.end(), options.begin(), options.end());
                }
                lines.push_back("Pilih petak properti milikmu, lalu klik tombol Festival.");
                showOverlay("Festival", lines);
                return;
            }

            if (hasBlockingPropertyDecision()) {
                showOverlay(
                    "Pilih Aksi Properti",
                    {
                        "Properti " + displayName(pendingProperty->getName()) + " (" + pendingProperty->getCode() + ") belum diproses.",
                        "Pilih Beli atau Lelang sebelum mengakhiri giliran."
                    }
                );
                return;
            }

            if (!g->getHasRolledThisTurn()) {
                showOverlay(
                    "Giliran Belum Selesai",
                    {"Silakan lempar dadu sebelum mengakhiri giliran."}
                );
                return;
            }

            if (g->isExtraRollPending()) {
                if (g->prepareExtraRollForCurrentPlayer()) {
                    propertyDecisionPending = false;
                    propertyDecisionResolved = false;
                    pendingProperty = nullptr;
                    showOverlay(
                        "Double",
                        {
                            "Pemain mendapat giliran ekstra.",
                            "Lempar dadu lagi untuk melanjutkan giliran."
                        }
                    );
                    return;
                }
            }

            try {
                g->endTurn();
            } catch (const HandOverflowException& e) {
                propertyDecisionPending = false;
                propertyDecisionResolved = false;
                pendingProperty = nullptr;
                openCardModal(true);
                cardError = e.getMessage();
                return;
            }

            propertyDecisionPending = false;
            propertyDecisionResolved = false;
            pendingProperty = nullptr;

            if (g->isGameOver()) {
                showOverlay(
                    "Permainan Selesai",
                    buildRankingLines(g),
                    "Batas maksimum turn telah tercapai."
                );
                return;
            }

            showOverlay(
                "Giliran Selesai",
                {
                    "Giliran berpindah ke pemain berikutnya."
                }
            );
        }},

        // {"Kemenangan", [this]() {
        //     Game* g = gameManager->getCurrentGame();
        //     if (g == nullptr) return;

        //     std::vector<Player*> sorted;
        //     for (Player& p : g->getPlayers()) {
        //         sorted.push_back(&p);
        //     }

        //     std::sort(sorted.begin(), sorted.end(), [](Player* a, Player* b) {
        //         return a->getTotalWealth() > b->getTotalWealth();
        //     });

        //     std::vector<std::string> lines;
        //     lines.push_back("=== Klasemen Sementara ===");

        //     for (std::size_t i = 0; i < sorted.size(); ++i) {
        //         lines.push_back(
        //             std::to_string(i + 1) + ". " +
        //             sorted[i]->getUsername() +
        //             " - M" + std::to_string(sorted[i]->getTotalWealth()) +
        //             (sorted[i]->isBankrupt() ? " [BANGKRUT]" : "")
        //         );
        //     }

        //     showOverlay(
        //         "Kemenangan",
        //         lines,
        //         "Pemenang final ditentukan berdasarkan kondisi gameOver / max turn."
        //     );
        // }},

        {"Kemenangan", [this]() {
            Game* g = gameManager->getCurrentGame();

            if (g == nullptr) {
                return;
            }

            showOverlay(
                g->isGameOver() ? "Kemenangan" : "Klasemen Sementara",
                buildRankingLines(g),
                g->isGameOver()
                    ? "Game sudah selesai."
                    : "Game belum selesai, ini hanya klasemen sementara."
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
    diceOneField.setMaxLength(4);
    diceTwoField.setMaxLength(4);
    auctionBidField.setMaxLength(12);
    tradeTargetField.setMaxLength(24);
    tradeOfferPropsField.setMaxLength(96);
    tradeOfferMoneyField.setMaxLength(12);
    tradeRequestPropsField.setMaxLength(96);
    tradeRequestMoneyField.setMaxLength(12);

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

    openLogButton.setOnClick([this]() {
        showLogModal = true;
        overlayOpen = false;
    });

    closeLogButton.setOnClick([this]() {
        showLogModal = false;
    });

    closeCardButton.setOnClick([this]() {
        if (cardOverflowMode) {
            cardError = "Buang satu kartu terlebih dahulu.";
            return;
        }

        showCardModal = false;
        selectedCardIndex = -1;
        cardError.clear();
    });

    useCardButton.setOnClick([this]() {
        onUseSelectedCard();
    });

    discardCardButton.setOnClick([this]() {
        onDiscardSelectedCard();
    });

    cancelLassoTargetButton.setOnClick([this]() {
        showLassoTargetModal = false;
        lassoTargetPlayerIds.clear();
        lassoTargetButtons.clear();
        pendingLassoCardIndex = -1;
    });

    auctionBidButton.setOnClick([this]() {
        onAuctionBid();
    });

    auctionPassButton.setOnClick([this]() {
        onAuctionPass();
    });

    auctionCloseButton.setOnClick([this]() {
        finishAuctionNotice();
    });

    tradeSendButton.setOnClick([this]() {
        onTradeSubmit();
    });

    tradeCancelButton.setOnClick([this]() {
        showTradeModal = false;
        tradeResponseMode = false;
        pendingTrade = nullptr;
        tradeError.clear();
    });

    tradeAcceptButton.setOnClick([this]() {
        onTradeAccept();
    });

    tradeRejectButton.setOnClick([this]() {
        onTradeReject();
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
    showLogModal = false;
    logModalVis = 0;
    showCardModal = false;
    cardOverflowMode = false;
    selectedCardIndex = -1;
    cardError.clear();
    cardModalVis = 0;
    showLassoTargetModal = false;
    lassoTargetPlayerIds.clear();
    lassoTargetButtons.clear();
    pendingLassoCardIndex = -1;
    lassoTargetModalVis = 0;
    propertyDecisionPending = false;
    propertyDecisionResolved = false;
    pendingProperty = nullptr;
    showAuctionModal = false;
    auctionNoticeMode = false;
    auctionError.clear();
    auctionNoticeLines.clear();
    auctionModalVis = 0;
    auctionBidField.setContent("");
    showTradeModal = false;
    tradeResponseMode = false;
    tradeError.clear();
    pendingTrade = nullptr;
    tradeModalVis = 0;
    tradeTargetField.setContent("");
    tradeOfferPropsField.setContent("");
    tradeOfferMoneyField.setContent("");
    tradeRequestPropsField.setContent("");
    tradeRequestMoneyField.setContent("");

    tokenPos.clear();
    tokenPhase.clear();
    selectedTile = 0;
}

void InGameScene::rollDiceAndShowResult() {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || g->isGameOver()) return;

    refreshPropertyDecisionState();
    if (hasBlockingPropertyDecision()) {
        showOverlay(
            "Pilih Aksi Properti",
            {
                "Properti " + displayName(pendingProperty->getName()) + " (" + pendingProperty->getCode() + ") belum diproses.",
                "Pilih Beli atau Lelang sebelum melempar dadu."
            }
        );
        return;
    }

    if (g->getHasRolledThisTurn()) {
        showOverlay("Dadu", {"Dadu sudah digunakan pada giliran ini."});
        return;
    }

    auto result = g->rollDiceForCurrentPlayer();

    if (result.first == 0 && result.second == 0) {
        int idx = g->getTurnManager().getCurrentPlayerIndex();
        if (idx >= 0 && idx < static_cast<int>(g->getPlayers().size()) &&
            g->getPlayer(idx).getStatus() == PlayerStatus::JAILED &&
            g->getPlayer(idx).getJailTurnsAttempted() >= 3) {
            showOverlay(
                "Penjara",
                {"Sudah gagal 3 kali. Pemain wajib membayar denda penjara sebelum melempar dadu."}
            );
        } else {
            showOverlay("Dadu", {"Dadu tidak bisa dilempar saat ini."});
        }
        return;
    }

    propertyDecisionResolved = false;
    refreshPropertyDecisionState();

    if (g->isFestivalSelectionPendingForCurrentPlayer()) {
        int idx = g->getTurnManager().getCurrentPlayerIndex();
        if (idx >= 0 && idx < static_cast<int>(g->getPlayers().size())) {
            std::vector<std::string> lines = {
                "Hasil dadu: " + std::to_string(result.first) + " + " + std::to_string(result.second),
                "Kamu mendarat di petak Festival!"
            };
            const std::vector<std::string> options = g->buildFestivalSelectionLines(g->getPlayer(idx));
            lines.insert(lines.end(), options.begin(), options.end());
            lines.push_back("Pilih petak properti milikmu, lalu klik tombol Festival.");
            showOverlay("Festival", lines);
        }
        return;
    }

    if (propertyDecisionPending && pendingProperty != nullptr) {
        int idx = g->getTurnManager().getCurrentPlayerIndex();
        if (idx >= 0 && idx < static_cast<int>(g->getPlayers().size()) &&
            !g->getPlayer(idx).canAfford(pendingProperty->getLandPrice())) {
            startAuctionForProperty(pendingProperty, "Uang pemain tidak cukup untuk membeli properti.");
            return;
        }

        showOverlay(
            "Properti Tersedia",
            {
                "Hasil dadu: " + std::to_string(result.first) + " + " + std::to_string(result.second),
                displayName(pendingProperty->getName()) + " (" + pendingProperty->getCode() + ") belum dimiliki.",
                "Harga beli: " + formatMoney(pendingProperty->getLandPrice()),
                "Pilih Beli atau Lelang sebelum mengakhiri giliran."
            }
        );
        return;
    }

    showOverlay(
        "Dadu",
        g->isExtraRollPending()
            ? std::vector<std::string>{
                "Hasil: " + std::to_string(result.first) + " + " + std::to_string(result.second),
                "Total langkah: " + std::to_string(result.first + result.second),
                "Double! Pilih Akhir Giliran untuk menyiapkan roll ekstra."
              }
            : std::vector<std::string>{
            "Hasil: " + std::to_string(result.first) + " + " + std::to_string(result.second),
            "Total langkah: " + std::to_string(result.first + result.second)
              }
    );
}

void InGameScene::onManualDiceSubmit() {
    auto parseDie = [](const std::string& text, int& value) -> bool {
        if (text.empty()) return false;
        std::size_t start = 0;
        if (text[0] == '-') {
            if (text.size() == 1) return false;
            start = 1;
        }
        for (std::size_t i = start; i < text.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(text[i]))) return false;
        }
        try {
            value = std::stoi(text);
        } catch (...) {
            return false;
        }
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

void InGameScene::refreshPropertyDecisionState() {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || g->isGameOver() || g->getAuctionManager().isAuctionActive()) {
        return;
    }

    if (propertyDecisionPending) {
        if (pendingProperty == nullptr || pendingProperty->getStatus() != BANK) {
            propertyDecisionPending = false;
            propertyDecisionResolved = true;
            pendingProperty = nullptr;
        }
        return;
    }

    if (propertyDecisionResolved || g->getBoard().size() == 0) {
        return;
    }

    int idx = g->getTurnManager().getCurrentPlayerIndex();
    if (idx < 0 || idx >= static_cast<int>(g->getPlayers().size())) {
        return;
    }

    StreetTile* property = dynamic_cast<StreetTile*>(
        g->getBoard().getTileByIndex(g->getPlayer(idx).getPosition())
    );

    if (property != nullptr && property->getStatus() == BANK) {
        propertyDecisionPending = true;
        pendingProperty = property;
    }
}

bool InGameScene::hasBlockingPropertyDecision() const {
    return propertyDecisionPending && pendingProperty != nullptr &&
           pendingProperty->getStatus() == BANK;
}

void InGameScene::startAuctionForProperty(PropertyTile* property, const std::string& reason) {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || property == nullptr || property->getStatus() != BANK) {
        return;
    }

    int idx = g->getTurnManager().getCurrentPlayerIndex();
    if (idx < 0 || idx >= static_cast<int>(g->getPlayers().size())) {
        return;
    }

    std::vector<Player*> bidders;
    for (Player& player : g->getPlayers()) {
        if (!player.isBankrupt() && player.canAfford(0)) {
            bidders.push_back(&player);
        }
    }

    g->getAuctionManager().runAuction(*property, bidders, g->getPlayer(idx), *g);
    propertyDecisionPending = false;
    propertyDecisionResolved = true;
    pendingProperty = nullptr;
    overlayOpen = false;
    auctionError.clear();
    auctionBidField.setContent("");
    showAuctionModal = true;

    if (!g->getAuctionManager().isAuctionActive()) {
        auctionNoticeMode = true;
        auctionNoticeLines = buildAuctionResultLines(g->getAuctionManager());
        if (!reason.empty()) {
            auctionNoticeLines.insert(auctionNoticeLines.begin(), reason);
        }
        return;
    }

    auctionNoticeMode = false;
    auctionNoticeLines.clear();
}

void InGameScene::onAuctionBid() {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr) return;

    AuctionManager& auction = g->getAuctionManager();
    if (!auction.isAuctionActive()) {
        return;
    }

    std::string text = trimCopy(auctionBidField.getContent());
    if (text.empty()) {
        auctionError = "Masukkan jumlah bid.";
        return;
    }

    for (char c : text) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            auctionError = "Bid harus berupa angka.";
            return;
        }
    }

    int amount = 0;
    try {
        amount = std::stoi(text);
    } catch (const std::exception&) {
        auctionError = "Jumlah bid tidak valid.";
        return;
    }

    Player* bidder = auction.getCurrentTurnPlayer();
    if (bidder == nullptr) {
        auctionError = "Tidak ada pemain yang sedang mendapat giliran.";
        return;
    }

    if (amount < auction.getMinimumBid()) {
        auctionError = "Bid minimal " + formatMoney(auction.getMinimumBid()) + ".";
        return;
    }

    if (!bidder->canAfford(amount)) {
        auctionError = bidder->getUsername() + " tidak memiliki uang cukup.";
        return;
    }

    if (!auction.processAction("BID", amount)) {
        auctionError = "Bid tidak valid.";
        return;
    }

    g->getLogger().log(
        g->getTurnManager().getCurrentTurn(),
        bidder->getUsername(),
        "LELANG_BID",
        formatMoney(amount)
    );

    auctionError.clear();
    auctionBidField.setContent("");

    if (!auction.isAuctionActive()) {
        auctionNoticeMode = true;
        auctionNoticeLines = buildAuctionResultLines(auction);
    }
}

void InGameScene::onAuctionPass() {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr) return;

    AuctionManager& auction = g->getAuctionManager();
    if (!auction.isAuctionActive()) {
        return;
    }

    Player* bidder = auction.getCurrentTurnPlayer();
    std::string bidderName = bidder != nullptr ? bidder->getUsername() : "Unknown";

    if (!auction.processAction("PASS")) {
        auctionError = "PASS tidak valid.";
        return;
    }

    g->getLogger().log(
        g->getTurnManager().getCurrentTurn(),
        bidderName,
        "LELANG_PASS",
        "Pass"
    );

    auctionError.clear();
    auctionBidField.setContent("");

    if (!auction.isAuctionActive()) {
        auctionNoticeMode = true;
        auctionNoticeLines = buildAuctionResultLines(auction);
    }
}

void InGameScene::finishAuctionNotice() {
    if (!auctionNoticeMode) {
        return;
    }

    showAuctionModal = false;
    auctionNoticeMode = false;
    auctionError.clear();
    auctionNoticeLines.clear();
    auctionBidField.setContent("");
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

void InGameScene::openTradeProposal() {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || g->isGameOver()) {
        return;
    }

    if (g->getAuctionManager().isAuctionActive() || showAuctionModal) {
        showAuctionModal = true;
        overlayOpen = false;
        return;
    }

    refreshPropertyDecisionState();
    if (hasBlockingPropertyDecision()) {
        showOverlay(
            "Trade Ditunda",
            {
                "Selesaikan keputusan properti terlebih dahulu.",
                "Pilih Beli atau Lelang sebelum melakukan trade."
            }
        );
        return;
    }

    tradeTargetField.setContent("");
    tradeOfferPropsField.setContent("");
    tradeOfferMoneyField.setContent("");
    tradeRequestPropsField.setContent("");
    tradeRequestMoneyField.setContent("");
    tradeError.clear();
    pendingTrade = nullptr;
    tradeResponseMode = false;
    showTradeModal = true;
    overlayOpen = false;
}

void InGameScene::onTradeSubmit() {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || g->isGameOver()) {
        return;
    }

    int currentIndex = g->getTurnManager().getCurrentPlayerIndex();
    if (currentIndex < 0 || currentIndex >= static_cast<int>(g->getPlayers().size())) {
        tradeError = "Pemain aktif tidak valid.";
        return;
    }

    Player* proposer = &g->getPlayer(currentIndex);
    Player* target = findPlayerByUsername(g, tradeTargetField.getContent());
    if (target == nullptr) {
        tradeError = "Nama player lawan tidak valid.";
        return;
    }

    if (target == proposer) {
        tradeError = "Tidak bisa trade dengan diri sendiri.";
        return;
    }

    if (target->isBankrupt()) {
        tradeError = "Tidak bisa trade dengan player bangkrut.";
        return;
    }

    int offeredMoney = 0;
    int requestedMoney = 0;
    std::vector<PropertyTile*> offeredProps;
    std::vector<PropertyTile*> requestedProps;
    std::string error;

    if (!parseMoneyField(tradeOfferMoneyField.getContent(), offeredMoney, error, "Uang diberikan") ||
        !parseMoneyField(tradeRequestMoneyField.getContent(), requestedMoney, error, "Uang diminta") ||
        !parsePropertyList(g, tradeOfferPropsField.getContent(), offeredProps, error, "properti diberikan") ||
        !parsePropertyList(g, tradeRequestPropsField.getContent(), requestedProps, error, "properti diminta")) {
        tradeError = error;
        return;
    }

    if (offeredMoney == 0 && requestedMoney == 0 && offeredProps.empty() && requestedProps.empty()) {
        tradeError = "Trade harus berisi uang atau properti.";
        return;
    }

    if (!proposer->canAfford(offeredMoney)) {
        tradeError = "Uang yang diberikan melebihi saldo player aktif.";
        return;
    }

    if (!target->canAfford(requestedMoney)) {
        tradeError = "Uang yang diminta melebihi saldo player lawan.";
        return;
    }

    for (PropertyTile* property : offeredProps) {
        if (property->getOwner() != proposer) {
            tradeError = "Properti " + property->getCode() + " bukan milik player aktif.";
            return;
        }

        StreetTile* street = dynamic_cast<StreetTile*>(property);
        if (street != nullptr && street->getBuildingLevel() > 0) {
            tradeError = "Properti " + property->getCode() + " masih memiliki bangunan.";
            return;
        }
    }

    for (PropertyTile* property : requestedProps) {
        if (property->getOwner() != target) {
            tradeError = "Properti " + property->getCode() + " bukan milik player lawan.";
            return;
        }

        StreetTile* street = dynamic_cast<StreetTile*>(property);
        if (street != nullptr && street->getBuildingLevel() > 0) {
            tradeError = "Properti " + property->getCode() + " masih memiliki bangunan.";
            return;
        }
    }

    try {
        pendingTrade = g->getTradeManager().proposeTrade(
            proposer,
            target,
            offeredProps,
            offeredMoney,
            requestedProps,
            requestedMoney
        );

        tradeError.clear();
        tradeResponseMode = true;
    } catch (const std::exception& e) {
        tradeError = std::string("Trade gagal: ") + e.what();
    }
}

void InGameScene::onTradeAccept() {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || pendingTrade == nullptr) {
        showTradeModal = false;
        tradeResponseMode = false;
        pendingTrade = nullptr;
        return;
    }

    std::vector<std::string> lines = buildTradeLines(pendingTrade);

    try {
        g->getTradeManager().acceptTrade(pendingTrade);
        pendingTrade = nullptr;
        showTradeModal = false;
        tradeResponseMode = false;
        tradeError.clear();
        lines.push_back("Trade diterima. Kepemilikan dan uang sudah ditukar.");
        showOverlay("Trade Diterima", lines);
    } catch (const std::exception& e) {
        pendingTrade = nullptr;
        showTradeModal = false;
        tradeResponseMode = false;
        tradeError.clear();
        showOverlay("Trade Gagal", {std::string(e.what())});
    }
}

void InGameScene::onTradeReject() {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || pendingTrade == nullptr) {
        showTradeModal = false;
        tradeResponseMode = false;
        pendingTrade = nullptr;
        return;
    }

    std::vector<std::string> lines = buildTradeLines(pendingTrade);
    g->getTradeManager().rejectTrade(pendingTrade);
    pendingTrade = nullptr;
    showTradeModal = false;
    tradeResponseMode = false;
    tradeError.clear();
    lines.push_back("Trade ditolak. Tidak ada perubahan.");
    showOverlay("Trade Ditolak", lines);
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
    logModalVis = ease(logModalVis, showLogModal ? 1.f : 0.f, dt * 8);
    cardModalVis = ease(cardModalVis, showCardModal ? 1.f : 0.f, dt * 8);
    lassoTargetModalVis = ease(lassoTargetModalVis, showLassoTargetModal ? 1.f : 0.f, dt * 8);
    auctionModalVis = ease(auctionModalVis, showAuctionModal ? 1.f : 0.f, dt * 8);
    tradeModalVis = ease(tradeModalVis, showTradeModal ? 1.f : 0.f, dt * 8);
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
        if (showLassoTargetModal) {
            showLassoTargetModal = false;
            lassoTargetPlayerIds.clear();
            lassoTargetButtons.clear();
            pendingLassoCardIndex = -1;
            return;
        }

        if (showAuctionModal && auctionNoticeMode) {
            finishAuctionNotice();
            return;
        }

        if (showTradeModal) {
            if (tradeResponseMode) {
                onTradeReject();
            } else {
                showTradeModal = false;
                tradeError.clear();
                pendingTrade = nullptr;
            }
            return;
        }

        if (showLogModal) {
            showLogModal = false;
            return;
        }

        if (showCardModal) {
            if (cardOverflowMode) {
                cardError = "Buang satu kartu terlebih dahulu.";
            } else {
                showCardModal = false;
                selectedCardIndex = -1;
                cardError.clear();
            }
            return;
        }

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

    bool noModalActive = saveModalVis < .02f && diceModalVis < .02f && logModalVis < .02f &&
                         cardModalVis < .02f &&
                         lassoTargetModalVis < .02f &&
                         auctionModalVis < .02f && tradeModalVis < .02f && overlayVis < .02f;

    if (noModalActive && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();

        for (size_t i = 0; i < tileRects.size(); ++i) {
            if (CheckCollisionPointRec(m, tileRects[i])) {
                selectedTile = static_cast<int>(i);
                break;
            }
        }
    }

    backToMenuBtn.setBoundary({sr.width - 130, 22, 106, 42});

    if (noModalActive) {
        backToMenuBtn.update();
    }

    float bw = (sb.width - 14) * .5f;
    float bh = 40;
    float sy = sb.y + 232;
    float rowGap = 46;
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
            sy + r * rowGap,
            bw,
            bh
        });

        if (noModalActive) {
            actionButtons[i].update();
        }
    }

    openLogButton.setBoundary({
        sb.x + 16,
        sb.y + 800,
        sb.width - 32,
        46
    });

    if (noModalActive) {
        openLogButton.update();
    }

    if (overlayVis > .01f) {
        Rectangle p{
            sr.width * .5f - 330,
            sr.height * .5f - 292 + (1 - overlayVis) * 24,
            660,
            584
        };

        closeOverlayBtn.setBoundary({
            p.x + p.width - 62,
            p.y + 12,
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

    if (logModalVis > .01f) {
        Rectangle p{
            sr.width * .5f - 340,
            sr.height * .5f - 302 + (1 - logModalVis) * 24,
            680,
            604
        };

        closeLogButton.setBoundary({
            p.x + p.width - 62,
            p.y + 12,
            50,
            38
        });

        closeLogButton.update();
    }

    if (cardModalVis > .01f) {
        Rectangle p{
            sr.width * .5f - 360,
            sr.height * .5f - 300 + (1 - cardModalVis) * 24,
            720,
            600
        };

        closeCardButton.setBoundary({
            p.x + p.width - 188,
            p.y + p.height - 70,
            140,
            50
        });

        discardCardButton.setBoundary({
            p.x + p.width - 354,
            p.y + p.height - 70,
            140,
            50
        });

        useCardButton.setBoundary({
            p.x + p.width - 520,
            p.y + p.height - 70,
            140,
            50
        });

        Game* currentGame = gameManager->getCurrentGame();
        int handSize = 0;
        if (currentGame != nullptr) {
            int idx = currentGame->getTurnManager().getCurrentPlayerIndex();
            if (idx >= 0 && idx < static_cast<int>(currentGame->getPlayers().size())) {
                handSize = static_cast<int>(currentGame->getPlayer(idx).getHandCards().size());
            }
        }

        if (selectedCardIndex >= handSize) {
            selectedCardIndex = handSize - 1;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = GetMousePosition();
            for (int i = 0; i < handSize; ++i) {
                Rectangle row{
                    p.x + 24,
                    p.y + 104 + i * 82.0f,
                    p.width - 48,
                    68
                };

                if (CheckCollisionPointRec(m, row)) {
                    selectedCardIndex = i;
                    cardError.clear();
                    break;
                }
            }
        }

        useCardButton.disabled = cardOverflowMode || selectedCardIndex < 0 || handSize == 0 ||
            (currentGame != nullptr && currentGame->getHasRolledThisTurn());
        discardCardButton.disabled = selectedCardIndex < 0 || handSize == 0;

        if (!cardOverflowMode) {
            useCardButton.update();
        }
        discardCardButton.update();
        closeCardButton.update();
    }

    if (lassoTargetModalVis > .01f) {
        Rectangle p{
            sr.width * .5f - 250,
            sr.height * .5f - 190 + (1 - lassoTargetModalVis) * 24,
            500,
            380
        };

        for (std::size_t i = 0; i < lassoTargetButtons.size(); ++i) {
            lassoTargetButtons[i].setBoundary({
                p.x + 34,
                p.y + 102 + static_cast<float>(i) * 58.0f,
                p.width - 68,
                46
            });
            lassoTargetButtons[i].update();
        }

        cancelLassoTargetButton.setBoundary({
            p.x + p.width - 174,
            p.y + p.height - 66,
            140,
            46
        });
        cancelLassoTargetButton.update();
    }

    if (auctionModalVis > .01f) {
        Rectangle p{
            sr.width * .5f - 330,
            sr.height * .5f - 246 + (1 - auctionModalVis) * 24,
            660,
            492
        };

        if (auctionNoticeMode) {
            auctionCloseButton.setBoundary({
                p.x + p.width - 188,
                p.y + p.height - 70,
                140,
                50
            });
            auctionCloseButton.update();
        } else {
            Game* currentGame = gameManager->getCurrentGame();
            if (currentGame != nullptr) {
                auctionBidButton.disabled = !currentGame->getAuctionManager().currentPlayerCanBid();
                auctionPassButton.disabled = !currentGame->getAuctionManager().currentPlayerCanPass();
            }

            auctionBidField.setBoundary({
                p.x + 24,
                p.y + p.height - 130,
                p.width - 220,
                48
            });
            auctionBidButton.setBoundary({
                p.x + p.width - 176,
                p.y + p.height - 130,
                152,
                48
            });
            auctionPassButton.setBoundary({
                p.x + p.width - 176,
                p.y + p.height - 72,
                152,
                48
            });

            auctionBidField.update();
            auctionBidButton.update();
            auctionPassButton.update();
        }
    }

    if (tradeModalVis > .01f) {
        Rectangle p{
            sr.width * .5f - 360,
            sr.height * .5f - 330 + (1 - tradeModalVis) * 24,
            720,
            660
        };

        if (tradeResponseMode) {
            tradeAcceptButton.setBoundary({
                p.x + p.width - 352,
                p.y + p.height - 72,
                146,
                50
            });

            tradeRejectButton.setBoundary({
                p.x + p.width - 182,
                p.y + p.height - 72,
                134,
                50
            });

            tradeAcceptButton.update();
            tradeRejectButton.update();
        } else {
            tradeTargetField.setBoundary({p.x + 24, p.y + 112, p.width - 48, 46});
            tradeOfferPropsField.setBoundary({p.x + 24, p.y + 238, p.width - 260, 46});
            tradeOfferMoneyField.setBoundary({p.x + p.width - 212, p.y + 238, 188, 46});
            tradeRequestPropsField.setBoundary({p.x + 24, p.y + 374, p.width - 260, 46});
            tradeRequestMoneyField.setBoundary({p.x + p.width - 212, p.y + 374, 188, 46});

            tradeSendButton.setBoundary({
                p.x + p.width - 204,
                p.y + p.height - 72,
                156,
                50
            });

            tradeCancelButton.setBoundary({
                p.x + 48,
                p.y + p.height - 72,
                140,
                50
            });

            tradeTargetField.update();
            tradeOfferPropsField.update();
            tradeOfferMoneyField.update();
            tradeRequestPropsField.update();
            tradeRequestMoneyField.update();
            tradeSendButton.update();
            tradeCancelButton.update();
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
    const char* title = "NIMONSPOLI";
    const int titleSize = 38;
    const float titleAngle = -22.0f * 3.14159265f / 180.0f;
    float titleX = rib.x + (rib.width - MeasureText(title, titleSize)) * .5f;
    float titleCenterX = rib.x + rib.width * .5f;
    float titleCenterY = rib.y + rib.height * .5f - titleSize * .5f;

    for (const char* ch = title; *ch != '\0'; ++ch) {
        char letter[2] = {*ch, '\0'};
        float offsetX = titleX - titleCenterX;
        DrawText(
            letter,
            static_cast<int>(titleX),
            static_cast<int>(titleCenterY + offsetX * std::tan(titleAngle)),
            titleSize,
            kText
        );
        titleX += MeasureText(letter, titleSize) + 4;
    }
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
    std::string moneyStr = "M" + std::to_string(cur.getMoney());
    DrawText(moneyStr.c_str(), int(sb.x + 16), int(sb.y + 88), 22, kAccentAlt);

    if (g->getBoard().size() > 0 && selectedTile >= 0 && selectedTile < g->getBoard().size()) {
        Tile* selected = g->getBoard().getTileByIndex(selectedTile);

        if (selected != nullptr) {
            DrawText("Petak Dipilih", int(sb.x + 16), int(sb.y + 116), 18, kText);
            std::string tileInfo = selected->getCode() + " - " + selected->getName();
            DrawText(
                tileInfo.c_str(),
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

                std::string statusText = "Status: " + status;
                DrawText(
                    statusText.c_str(),
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

    openLogButton.draw();
}

void InGameScene::drawOverlay(Rectangle sr) {
    if (overlayVis <= .01f || !overlayOpen) return;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(kText, .38f * overlayVis));

    float rise = (1 - overlayVis) * 24;
    Rectangle p{
        sr.width * .5f - 330,
        sr.height * .5f - 292 + rise,
        660,
        584
    };

    DrawRectangleRounded({p.x + 5, p.y + 9, p.width, p.height}, .09f, 10, Fade(kText, .12f * overlayVis));
    DrawRectangleRounded(p, .09f, 10, Fade({250,255,235,255}, overlayVis));
    DrawRectangleRoundedLinesEx(p, .09f, 10, 2.5f, Fade(kPanelBorder, overlayVis));

    drawSmallFlower(p.x + p.width - 28, p.y + 28, 14, sceneTime * .5f, .5f * overlayVis);

    DrawText(fitText(overlayTitle, 32, static_cast<int>(p.width - 100)).c_str(), int(p.x + 22), int(p.y + 20), 32, kText);

    closeOverlayBtn.draw();

    float y = p.y + 80;
    int fontSize = overlayLines.size() > 14U ? 17 : 20;
    float lineStep = overlayLines.size() > 14U ? 24.0f : 32.0f;
    int maxTextWidth = static_cast<int>(p.width - 48);

    for (const auto& line : overlayLines) {
        DrawText(fitText(line, fontSize, maxTextWidth).c_str(), int(p.x + 24), int(y), fontSize, kSubtext);
        y += lineStep;

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

void InGameScene::drawLogModal(Rectangle sr) {
    if (logModalVis <= .01f || !showLogModal) return;

    Game* g = gameManager->getCurrentGame();
    if (g == nullptr) return;

    DrawRectangle(
        0,
        0,
        GetScreenWidth(),
        GetScreenHeight(),
        Fade(kText, .38f * logModalVis)
    );

    Rectangle p{
        sr.width * .5f - 340,
        sr.height * .5f - 302 + (1 - logModalVis) * 24,
        680,
        604
    };

    DrawRectangleRounded(
        {p.x + 5, p.y + 9, p.width, p.height},
        .09f,
        10,
        Fade(kText, .12f * logModalVis)
    );

    DrawRectangleRounded(
        p,
        .09f,
        10,
        Fade({250,255,235,255}, logModalVis)
    );

    DrawRectangleRoundedLinesEx(
        p,
        .09f,
        10,
        2.5f,
        Fade(kPanelBorder, logModalVis)
    );

    drawSmallFlower(p.x + p.width - 28, p.y + 28, 14, sceneTime * .5f, .5f * logModalVis);

    DrawText(
        "Log Permainan",
        static_cast<int>(p.x + 24),
        static_cast<int>(p.y + 24),
        32,
        kText
    );

    closeLogButton.setBoundary({
        p.x + p.width - 62,
        p.y + 12,
        50,
        38
    });
    closeLogButton.draw();

    auto entries = g->getLogger().getEntries();
    Rectangle listArea{
        p.x + 24,
        p.y + 78,
        p.width - 48,
        p.height - 104
    };

    if (entries.empty()) {
        DrawText(
            "(belum ada log)",
            static_cast<int>(listArea.x),
            static_cast<int>(listArea.y + 8),
            20,
            kSubtext
        );
        return;
    }

    int logCount = std::min<int>(10, static_cast<int>(entries.size()));
    int maxTextWidth = static_cast<int>(listArea.width - 28);

    for (int i = 0; i < logCount; ++i) {
        const LogEntry& e = entries[entries.size() - 1 - static_cast<size_t>(i)];
        float rowY = listArea.y + i * 48;
        Rectangle row{
            listArea.x,
            rowY,
            listArea.width,
            40
        };

        DrawRectangleRounded(row, .18f, 8, Fade(kAccentAlt, i % 2 == 0 ? .08f : .04f));

        std::string header = "[T" + std::to_string(e.getTurn()) + "] " + e.getUsername() + " | " + e.getActionType();
        std::string detail = fitText(e.getDetail(), 17, maxTextWidth);

        DrawText(
            fitText(header, 18, maxTextWidth).c_str(),
            static_cast<int>(row.x + 14),
            static_cast<int>(row.y + 5),
            18,
            kText
        );

        DrawText(
            detail.c_str(),
            static_cast<int>(row.x + 14),
            static_cast<int>(row.y + 24),
            17,
            kSubtext
        );
    }
}

void InGameScene::openCardModal(bool overflowMode) {
    showCardModal = true;
    cardOverflowMode = overflowMode;
    selectedCardIndex = -1;
    cardError.clear();
    overlayOpen = false;
    showLogModal = false;
}

void InGameScene::onUseSelectedCard() {
    if (cardOverflowMode) return;

    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || selectedCardIndex < 0) {
        cardError = "Pilih kartu terlebih dahulu.";
        return;
    }

    refreshPropertyDecisionState();
    if (hasBlockingPropertyDecision()) {
        cardError = "Pilih Beli atau Lelang untuk properti saat ini terlebih dahulu.";
        return;
    }

    int currentIdx = g->getTurnManager().getCurrentPlayerIndex();
    if (currentIdx < 0 || currentIdx >= static_cast<int>(g->getPlayers().size())) {
        cardError = "Pemain aktif tidak valid.";
        return;
    }

    Player& current = g->getPlayer(currentIdx);
    const auto& hand = current.getHandCards();
    if (selectedCardIndex >= static_cast<int>(hand.size())) {
        cardError = "Kartu tidak valid.";
        return;
    }

    const std::string cardName = hand[static_cast<size_t>(selectedCardIndex)]->getName();

    try {
        bool ok = false;

        if (cardName == "TeleportCard") {
            ok = g->useCurrentPlayerTeleportCard(selectedCardIndex, selectedTile);
            if (!ok) {
                cardError = "Pilih petak tujuan yang valid sebelum memakai TeleportCard.";
                return;
            }
        } else if (cardName == "DemolitionCard") {
            ok = g->useCurrentPlayerDemolitionCard(selectedCardIndex, selectedTile);
            if (!ok) {
                cardError = "Pilih street milik lawan sebelum memakai DemolitionCard.";
                return;
            }
        } else if (cardName == "LassoCard") {
            std::vector<int> targets;
            for (Player& player : g->getPlayers()) {
                if (
                    player.getId() != current.getId() &&
                    !player.isBankrupt() &&
                    player.getPosition() == selectedTile
                ) {
                    targets.push_back(player.getId());
                }
            }

            if (targets.empty()) {
                cardError = "Pilih petak yang berisi pemain lawan untuk LassoCard.";
                return;
            }

            if (targets.size() > 1U) {
                openLassoTargetModal(targets);
                return;
            }

            ok = g->useCurrentPlayerLassoCard(selectedCardIndex, targets[0]);
            if (!ok) {
                cardError = "LassoCard tidak bisa dipakai pada target itu.";
                return;
            }
        } else {
            ok = g->useCurrentPlayerAbilityCard(selectedCardIndex);
        }

        if (!ok) {
            cardError = "Kartu tidak bisa dipakai saat ini.";
            return;
        }
    } catch (const std::exception& e) {
        cardError = e.what();
        return;
    }

    propertyDecisionResolved = false;
    refreshPropertyDecisionState();
    selectedCardIndex = -1;
    showCardModal = false;
    cardError.clear();
}

void InGameScene::onDiscardSelectedCard() {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || selectedCardIndex < 0) {
        cardError = "Pilih kartu terlebih dahulu.";
        return;
    }

    if (!g->discardCurrentPlayerAbilityCard(selectedCardIndex)) {
        cardError = "Kartu tidak bisa dibuang.";
        return;
    }

    int handSize = 0;
    int idx = g->getTurnManager().getCurrentPlayerIndex();
    if (idx >= 0 && idx < static_cast<int>(g->getPlayers().size())) {
        handSize = static_cast<int>(g->getPlayer(idx).getHandCards().size());
    }

    if (handSize <= 3) {
        cardOverflowMode = false;
    }

    selectedCardIndex = handSize > 0 ? std::min(selectedCardIndex, handSize - 1) : -1;
    cardError.clear();

    if (!cardOverflowMode && handSize == 0) {
        showCardModal = false;
    }
}

void InGameScene::openLassoTargetModal(const std::vector<int>& targetPlayerIds) {
    lassoTargetPlayerIds = targetPlayerIds;
    lassoTargetButtons.clear();
    pendingLassoCardIndex = selectedCardIndex;
    showLassoTargetModal = true;
    cardError.clear();

    Game* g = gameManager->getCurrentGame();

    for (int playerId : lassoTargetPlayerIds) {
        std::string label = "Pemain " + std::to_string(playerId);
        if (g != nullptr) {
            for (Player& player : g->getPlayers()) {
                if (player.getId() == playerId) {
                    label = player.getUsername();
                    break;
                }
            }
        }

        Button button(label, kAccentAlt, {255,255,255,255});
        button.setOnClick([this, playerId]() {
            onLassoTargetSelected(playerId);
        });
        lassoTargetButtons.push_back(button);
    }
}

void InGameScene::onLassoTargetSelected(int targetPlayerId) {
    Game* g = gameManager->getCurrentGame();
    if (g == nullptr || pendingLassoCardIndex < 0) {
        showLassoTargetModal = false;
        cardError = "Target Lasso tidak valid.";
        return;
    }

    try {
        if (!g->useCurrentPlayerLassoCard(pendingLassoCardIndex, targetPlayerId)) {
            cardError = "LassoCard tidak bisa dipakai pada target itu.";
            return;
        }
    } catch (const std::exception& e) {
        cardError = e.what();
        return;
    }

    showLassoTargetModal = false;
    lassoTargetPlayerIds.clear();
    lassoTargetButtons.clear();
    pendingLassoCardIndex = -1;
    propertyDecisionResolved = false;
    refreshPropertyDecisionState();
    selectedCardIndex = -1;
    showCardModal = false;
    cardError.clear();
}

void InGameScene::drawCardModal(Rectangle sr) {
    if (cardModalVis <= .01f || !showCardModal) return;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(kText, .38f * cardModalVis));

    Rectangle p{
        sr.width * .5f - 360,
        sr.height * .5f - 300 + (1 - cardModalVis) * 24,
        720,
        600
    };

    DrawRectangleRounded({p.x + 5, p.y + 9, p.width, p.height}, .09f, 10, Fade(kText, .12f * cardModalVis));
    DrawRectangleRounded(p, .09f, 10, Fade({250,255,235,255}, cardModalVis));
    DrawRectangleRoundedLinesEx(p, .09f, 10, 2.5f, Fade(kPanelBorder, cardModalVis));
    drawSmallFlower(p.x + p.width - 28, p.y + 28, 14, sceneTime * .5f, .5f * cardModalVis);

    DrawText(
        cardOverflowMode ? "Buang Kartu" : "Kartu Kemampuan",
        static_cast<int>(p.x + 24),
        static_cast<int>(p.y + 24),
        32,
        kText
    );

    DrawText(
        cardOverflowMode ? "Tangan penuh. Pilih satu kartu untuk dibuang." : "Pilih kartu, lalu pakai atau buang.",
        static_cast<int>(p.x + 24),
        static_cast<int>(p.y + 68),
        19,
        kSubtext
    );

    Game* g = gameManager->getCurrentGame();
    Player* current = nullptr;
    if (g != nullptr) {
        int idx = g->getTurnManager().getCurrentPlayerIndex();
        if (idx >= 0 && idx < static_cast<int>(g->getPlayers().size())) {
            current = &g->getPlayer(idx);
        }
    }

    if (current == nullptr || current->getHandCards().empty()) {
        DrawText(
            "(tidak ada kartu kemampuan)",
            static_cast<int>(p.x + 24),
            static_cast<int>(p.y + 122),
            22,
            kSubtext
        );
    } else {
        const auto& hand = current->getHandCards();
        for (std::size_t i = 0; i < hand.size(); ++i) {
            Rectangle row{
                p.x + 24,
                p.y + 104 + static_cast<float>(i) * 82.0f,
                p.width - 48,
                68
            };

            bool selected = static_cast<int>(i) == selectedCardIndex;
            DrawRectangleRounded(row, .13f, 8, Fade(selected ? kAccent : kAccentAlt, selected ? .22f : .08f));
            DrawRectangleRoundedLinesEx(row, .13f, 8, selected ? 2.5f : 1.5f, Fade(selected ? kAccent : kPanelBorder, cardModalVis));

            std::string title = std::to_string(i + 1) + ". " + hand[i]->getName();
            DrawText(
                fitText(title, 21, static_cast<int>(row.width - 28)).c_str(),
                static_cast<int>(row.x + 14),
                static_cast<int>(row.y + 10),
                21,
                kText
            );

            DrawText(
                fitText(hand[i]->getDescription(), 17, static_cast<int>(row.width - 28)).c_str(),
                static_cast<int>(row.x + 14),
                static_cast<int>(row.y + 38),
                17,
                kSubtext
            );
        }
    }

    if (!cardError.empty()) {
        DrawText(
            fitText(cardError, 18, static_cast<int>(p.width - 48)).c_str(),
            static_cast<int>(p.x + 24),
            static_cast<int>(p.y + p.height - 104),
            18,
            kDanger
        );
    }

    if (!cardOverflowMode) {
        useCardButton.setBoundary({
            p.x + p.width - 520,
            p.y + p.height - 70,
            140,
            50
        });
        useCardButton.draw();
    }

    discardCardButton.setBoundary({
        p.x + p.width - 354,
        p.y + p.height - 70,
        140,
        50
    });

    closeCardButton.setBoundary({
        p.x + p.width - 188,
        p.y + p.height - 70,
        140,
        50
    });

    discardCardButton.draw();
    closeCardButton.draw();
}

void InGameScene::drawLassoTargetModal(Rectangle sr) {
    if (lassoTargetModalVis <= .01f || !showLassoTargetModal) return;

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(kText, .26f * lassoTargetModalVis));

    Rectangle p{
        sr.width * .5f - 250,
        sr.height * .5f - 190 + (1 - lassoTargetModalVis) * 24,
        500,
        380
    };

    DrawRectangleRounded({p.x + 5, p.y + 9, p.width, p.height}, .09f, 10, Fade(kText, .12f * lassoTargetModalVis));
    DrawRectangleRounded(p, .09f, 10, Fade({250,255,235,255}, lassoTargetModalVis));
    DrawRectangleRoundedLinesEx(p, .09f, 10, 2.5f, Fade(kPanelBorder, lassoTargetModalVis));

    DrawText(
        "Pilih Target Lasso",
        static_cast<int>(p.x + 28),
        static_cast<int>(p.y + 24),
        30,
        kText
    );

    DrawText(
        "Ada beberapa pemain lawan di petak ini.",
        static_cast<int>(p.x + 28),
        static_cast<int>(p.y + 66),
        18,
        kSubtext
    );

    for (std::size_t i = 0; i < lassoTargetButtons.size(); ++i) {
        lassoTargetButtons[i].setBoundary({
            p.x + 34,
            p.y + 102 + static_cast<float>(i) * 58.0f,
            p.width - 68,
            46
        });
        lassoTargetButtons[i].draw();
    }

    cancelLassoTargetButton.setBoundary({
        p.x + p.width - 174,
        p.y + p.height - 66,
        140,
        46
    });
    cancelLassoTargetButton.draw();
}

void InGameScene::drawAuctionModal(Rectangle sr) {
    if (auctionModalVis <= .01f || !showAuctionModal) return;

    Game* g = gameManager->getCurrentGame();
    if (g == nullptr) return;

    AuctionManager& auction = g->getAuctionManager();

    DrawRectangle(
        0,
        0,
        GetScreenWidth(),
        GetScreenHeight(),
        Fade(kText, .38f * auctionModalVis)
    );

    Rectangle p{
        sr.width * .5f - 330,
        sr.height * .5f - 246 + (1 - auctionModalVis) * 24,
        660,
        492
    };

    DrawRectangleRounded({p.x + 5, p.y + 9, p.width, p.height}, .09f, 10, Fade(kText, .12f * auctionModalVis));
    DrawRectangleRounded(p, .09f, 10, Fade({250,255,235,255}, auctionModalVis));
    DrawRectangleRoundedLinesEx(p, .09f, 10, 2.5f, Fade(kPanelBorder, auctionModalVis));
    drawSmallFlower(p.x + p.width - 28, p.y + 28, 14, sceneTime * .5f, .5f * auctionModalVis);

    DrawText("Lelang Properti", static_cast<int>(p.x + 24), static_cast<int>(p.y + 24), 32, kText);

    if (auctionNoticeMode) {
        float y = p.y + 88;
        for (const std::string& line : auctionNoticeLines) {
            DrawText(
                fitText(line, 20, static_cast<int>(p.width - 48)).c_str(),
                static_cast<int>(p.x + 24),
                static_cast<int>(y),
                20,
                kSubtext
            );
            y += 34;
        }

        auctionCloseButton.setBoundary({
            p.x + p.width - 188,
            p.y + p.height - 70,
            140,
            50
        });
        auctionCloseButton.draw();
        return;
    }

    PropertyTile* property = auction.getCurrentProperty();
    Player* current = auction.getCurrentTurnPlayer();
    Player* highest = auction.getHighestBidder();

    std::string propertyLine = property != nullptr
        ? displayName(property->getName()) + " (" + property->getCode() + ") akan dilelang."
        : "Properti akan dilelang.";

    DrawText(
        fitText(propertyLine, 20, static_cast<int>(p.width - 48)).c_str(),
        static_cast<int>(p.x + 24),
        static_cast<int>(p.y + 82),
        20,
        kText
    );

    DrawText(
        ("Giliran: " + std::string(current != nullptr ? current->getUsername() : "-")).c_str(),
        static_cast<int>(p.x + 24),
        static_cast<int>(p.y + 124),
        24,
        kText
    );

    std::string highestLine = "Penawaran tertinggi: ";
    if (highest != nullptr) {
        highestLine += formatMoney(auction.getHighestBid()) + " (" + highest->getUsername() + ")";
    } else {
        highestLine += "Belum ada";
    }

    DrawText(highestLine.c_str(), static_cast<int>(p.x + 24), static_cast<int>(p.y + 164), 20, kSubtext);
    DrawText(
        ("Peserta aktif: " + std::to_string(auction.getActiveBidderCount())).c_str(),
        static_cast<int>(p.x + 24),
        static_cast<int>(p.y + 194),
        20,
        kSubtext
    );

    std::string ruleLine = "Bid minimal " + formatMoney(auction.getMinimumBid()) + ".";
    if (current != nullptr) {
        ruleLine += " Saldo " + current->getUsername() + ": " + formatMoney(current->getMoney()) + ".";
    }

    if (!auction.currentPlayerCanPass()) {
        ruleLine += " PASS tidak tersedia karena belum ada bid dan hanya pemain ini yang tersisa.";
    }

    DrawText(
        fitText(ruleLine, 18, static_cast<int>(p.width - 48)).c_str(),
        static_cast<int>(p.x + 24),
        static_cast<int>(p.y + 238),
        18,
        kSubtext
    );

    if (!auctionError.empty()) {
        DrawText(
            fitText(auctionError, 18, static_cast<int>(p.width - 48)).c_str(),
            static_cast<int>(p.x + 24),
            static_cast<int>(p.y + 274),
            18,
            kDanger
        );
    }

    auctionBidField.setBoundary({
        p.x + 24,
        p.y + p.height - 130,
        p.width - 220,
        48
    });
    auctionBidField.draw();

    auctionBidButton.disabled = !auction.currentPlayerCanBid();
    auctionBidButton.setBoundary({
        p.x + p.width - 176,
        p.y + p.height - 130,
        152,
        48
    });
    auctionBidButton.draw();

    auctionPassButton.disabled = !auction.currentPlayerCanPass();
    auctionPassButton.setBoundary({
        p.x + p.width - 176,
        p.y + p.height - 72,
        152,
        48
    });
    auctionPassButton.draw();
}

void InGameScene::drawTradeModal(Rectangle sr) {
    if (tradeModalVis <= .01f || !showTradeModal) return;

    DrawRectangle(
        0,
        0,
        GetScreenWidth(),
        GetScreenHeight(),
        Fade(kText, .38f * tradeModalVis)
    );

    Rectangle p{
        sr.width * .5f - 360,
        sr.height * .5f - 330 + (1 - tradeModalVis) * 24,
        720,
        660
    };

    DrawRectangleRounded({p.x + 5, p.y + 9, p.width, p.height}, .09f, 10, Fade(kText, .12f * tradeModalVis));
    DrawRectangleRounded(p, .09f, 10, Fade({250,255,235,255}, tradeModalVis));
    DrawRectangleRoundedLinesEx(p, .09f, 10, 2.5f, Fade(kPanelBorder, tradeModalVis));
    drawSmallFlower(p.x + p.width - 28, p.y + 28, 14, sceneTime * .5f, .5f * tradeModalVis);

    if (tradeResponseMode) {
        DrawText("Konfirmasi Trade", static_cast<int>(p.x + 24), static_cast<int>(p.y + 24), 32, kText);

        std::vector<std::string> lines = buildTradeLines(pendingTrade);
        float y = p.y + 92;

        for (const std::string& line : lines) {
            DrawText(
                fitText(line, 22, static_cast<int>(p.width - 48)).c_str(),
                static_cast<int>(p.x + 24),
                static_cast<int>(y),
                22,
                kSubtext
            );
            y += 42;
        }

        DrawText(
            "Player target dapat menerima atau menolak penawaran ini.",
            static_cast<int>(p.x + 24),
            static_cast<int>(p.y + 306),
            20,
            kText
        );

        if (!tradeError.empty()) {
            DrawText(
                fitText(tradeError, 18, static_cast<int>(p.width - 48)).c_str(),
                static_cast<int>(p.x + 24),
                static_cast<int>(p.y + 352),
                18,
                kDanger
            );
        }

        tradeAcceptButton.setBoundary({
            p.x + p.width - 352,
            p.y + p.height - 72,
            146,
            50
        });

        tradeRejectButton.setBoundary({
            p.x + p.width - 182,
            p.y + p.height - 72,
            134,
            50
        });

        tradeAcceptButton.draw();
        tradeRejectButton.draw();
        return;
    }

    DrawText("Ajukan Trade", static_cast<int>(p.x + 24), static_cast<int>(p.y + 24), 32, kText);
    DrawText(
        "Masukkan kode properti dipisah koma atau spasi. Field uang/properti boleh kosong.",
        static_cast<int>(p.x + 24),
        static_cast<int>(p.y + 70),
        18,
        kSubtext
    );

    DrawText("Player Lawan", static_cast<int>(p.x + 24), static_cast<int>(p.y + 88), 18, kText);
    tradeTargetField.setBoundary({p.x + 24, p.y + 112, p.width - 48, 46});
    tradeTargetField.draw();

    DrawText("Yang Diberikan", static_cast<int>(p.x + 24), static_cast<int>(p.y + 188), 22, kText);
    DrawText("Properti", static_cast<int>(p.x + 24), static_cast<int>(p.y + 216), 16, kSubtext);
    DrawText("Uang", static_cast<int>(p.x + p.width - 212), static_cast<int>(p.y + 216), 16, kSubtext);
    tradeOfferPropsField.setBoundary({p.x + 24, p.y + 238, p.width - 260, 46});
    tradeOfferMoneyField.setBoundary({p.x + p.width - 212, p.y + 238, 188, 46});
    tradeOfferPropsField.draw();
    tradeOfferMoneyField.draw();

    DrawText("Yang Diminta", static_cast<int>(p.x + 24), static_cast<int>(p.y + 324), 22, kText);
    DrawText("Properti", static_cast<int>(p.x + 24), static_cast<int>(p.y + 352), 16, kSubtext);
    DrawText("Uang", static_cast<int>(p.x + p.width - 212), static_cast<int>(p.y + 352), 16, kSubtext);
    tradeRequestPropsField.setBoundary({p.x + 24, p.y + 374, p.width - 260, 46});
    tradeRequestMoneyField.setBoundary({p.x + p.width - 212, p.y + 374, 188, 46});
    tradeRequestPropsField.draw();
    tradeRequestMoneyField.draw();

    if (!tradeError.empty()) {
        DrawText(
            fitText(tradeError, 18, static_cast<int>(p.width - 48)).c_str(),
            static_cast<int>(p.x + 24),
            static_cast<int>(p.y + 438),
            18,
            kDanger
        );
    }

    Game* g = gameManager->getCurrentGame();
    if (g != nullptr) {
        int currentIndex = g->getTurnManager().getCurrentPlayerIndex();
        float y = p.y + 472;
        DrawText("Player tersedia:", static_cast<int>(p.x + 24), static_cast<int>(y), 18, kText);
        y += 26;

        for (std::size_t i = 0; i < g->getPlayers().size(); ++i) {
            if (static_cast<int>(i) == currentIndex) {
                continue;
            }

            Player& player = g->getPlayers()[i];
            std::string row = player.getUsername() + " - " + formatMoney(player.getMoney());
            DrawText(
                fitText(row, 17, static_cast<int>(p.width - 48)).c_str(),
                static_cast<int>(p.x + 24),
                static_cast<int>(y),
                17,
                player.isBankrupt() ? kDanger : kSubtext
            );
            y += 24;
            if (y > p.y + p.height - 100) {
                break;
            }
        }
    }

    tradeSendButton.setBoundary({
        p.x + p.width - 204,
        p.y + p.height - 72,
        156,
        50
    });

    tradeCancelButton.setBoundary({
        p.x + 48,
        p.y + p.height - 72,
        140,
        50
    });

    tradeSendButton.draw();
    tradeCancelButton.draw();
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
    drawLogModal(sr);
    drawCardModal(sr);
    drawLassoTargetModal(sr);
    drawAuctionModal(sr);
    drawTradeModal(sr);
}
