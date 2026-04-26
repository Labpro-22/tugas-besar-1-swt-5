#include "../../include/models/ChanceCard.hpp"
#include "../../include/utils/Board.hpp"
#include "../../include/core/Game.hpp"
#include "../../include/models/AbilityCard.hpp"

StepbackCard::StepbackCard() : ChanceCard(
    "Mundur 3 petak.",
    [](Player* p, Game* g) {
        Board& board = g->getBoard();
        bool dummy = false;
        int stepbackIndex = board.calculateNewPosition(p->getPosition(), -3, dummy);
        p->moveTo(stepbackIndex);
        board.getTileByIndex(stepbackIndex)->onLand(p, g);
    }
) {}
RailroadCard::RailroadCard() : ChanceCard(
    "Pergi ke stasiun terdekat.",
    [](Player* p, Game* g) {
        Board& board = g->getBoard();
        int target = board.getStepsToNearestRailroad(p->getPosition());
        p->moveTo(target);
        board.getTileByIndex(target)->onLand(p, g);
    }
) {}
GoToJailCard::GoToJailCard() : ChanceCard(
    "Masuk Penjara.",
    [](Player* p, Game* g) {
        Board& board = g->getBoard();
        int jailIndex = board.getJailIndex();
        p->moveTo(jailIndex);
        p->enterJail();
    }
) {}
