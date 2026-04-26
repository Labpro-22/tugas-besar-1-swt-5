#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <map>

#include "../include/core/Game.hpp"
#include "../include/core/GameManager.hpp"
#include "../include/core/AccountManager.hpp"
#include "../include/data-layer/ConfigComposer.hpp"
#include "../include/utils/BoardBuilder.hpp"

using namespace std;

static const string CONFIG_DIR = "config/basic/";

static Config loadConfig() {
    ConfigComposer composer(
        CONFIG_DIR + "property.txt",
        CONFIG_DIR + "railroad.txt",
        CONFIG_DIR + "utility.txt",
        CONFIG_DIR + "tax.txt",
        CONFIG_DIR + "aksi.txt",
        CONFIG_DIR + "special.txt",
        CONFIG_DIR + "misc.txt"
    );
    return composer.getConfig();
}

static void buildBoard(Game* game, Config& cfg) {
    BoardBuilder::build(
        game->getBoard(),
        cfg.getPropertyConfigAll(),
        cfg.getActionTileConfigAll(),
        [&]() {
            map<int,int> m;
            for (int i = 1; i <= 4; ++i) m[i] = cfg.getRailroadRent(i);
            return m;
        }(),
        [&]() {
            map<int,int> m;
            for (int i = 1; i <= 2; ++i) m[i] = cfg.getUtilityMultiplier(i);
            return m;
        }()
    );
    if (game->getBoard().size() == 0) {
        throw runtime_error("Board kosong setelah build. Periksa file konfigurasi.");
    }
}

static void initNewGame(GameManager& gm, AccountManager& am, Config& cfg) {
    int numPlayers = 0;
    while (numPlayers < 2 || numPlayers > 4) {
        cout << "Masukkan jumlah pemain (2-4): ";
        cin >> numPlayers;
        if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); numPlayers = 0; }
    }
    cin.ignore(1000, '\n');

    vector<string> names;
    for (int i = 0; i < numPlayers; ++i) {
        string name;
        while (name.empty()) {
            cout << "Username pemain " << (i + 1) << ": ";
            getline(cin, name);
        }
        names.push_back(name);
    }

    gm.startNewGame();
    Game* game = gm.getCurrentGame();
    game->setConfig(cfg);

    buildBoard(game, cfg);

    int startMoney = cfg.getMiscConfig(SALDO_AWAL);
    if (startMoney <= 0) startMoney = 1500;
    int maxTurn = cfg.getMiscConfig(MAX_TURN);

    vector<int> order;
    for (int i = 0; i < numPlayers; ++i) {
        Account acc(names[i], "pass", 0);
        am.addAccount(acc);
        Account* accPtr = am.getAccount(names[i], "pass");
        game->getPlayers().emplace_back(i, accPtr, startMoney);
        order.push_back(i);
    }

    // Acak urutan giliran
    random_device rd;
    mt19937 rng(rd());
    shuffle(order.begin(), order.end(), rng);

    game->getTurnManager() = TurnManager(order, maxTurn);

    cout << "\n=== PERMAINAN DIMULAI ===\n";
    cout << "Board  : " << game->getBoard().size() << " petak\n";
    cout << "Pemain : " << numPlayers << "\n";
    cout << "Saldo  : M" << startMoney << "\n";
    cout << "Max turn: " << (maxTurn > 0 ? to_string(maxTurn) : "Tidak terbatas (bankruptcy)") << "\n";
    cout << "Urutan : ";
    for (int idx : order) cout << game->getPlayers()[idx].getUsername() << " ";
    cout << "\n\n";
}

int main() {
    cout << "============================================\n";
    cout << "         NIMONSPOLI - Board Game            \n";
    cout << "============================================\n\n";

    Config cfg;
    try {
        cfg = loadConfig();
        cout << "Konfigurasi berhasil dimuat.\n\n";
    } catch (const exception& e) {
        cerr << "Gagal memuat konfigurasi: " << e.what() << "\n";
        return 1;
    }

    GameManager gm;
    AccountManager am;

    cout << "1. New Game\n";
    cout << "2. Load Game\n";
    cout << "Pilihan: ";

    int choice = 0;
    cin >> choice;
    if (cin.fail()) { cin.clear(); cin.ignore(1000, '\n'); choice = 0; }

    try {
        if (choice == 1) {
            initNewGame(gm, am, cfg);
        } else if (choice == 2) {
            cin.ignore(1000, '\n');
            cout << "Masukkan nama file save: ";
            string filename;
            getline(cin, filename);
            gm.loadGame(filename);
            Game* game = gm.getCurrentGame();
            if (!game) { cerr << "Gagal memuat game.\n"; return 1; }
            game->setConfig(cfg);
            buildBoard(game, cfg);
            cout << "Game dimuat dari " << filename << ".\n\n";
        } else {
            cout << "Pilihan tidak valid.\n";
            return 1;
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    Game* game = gm.getCurrentGame();
    if (!game) { cerr << "Tidak ada game aktif.\n"; return 1; }

    // Jalankan game loop CLI
    game->run();

    return 0;
}