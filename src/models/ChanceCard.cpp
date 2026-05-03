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
        g->handleLanding(*p);
    }
) {}
RailroadCard::RailroadCard() : ChanceCard(
    "Pergi ke stasiun terdekat.",
    [](Player* p, Game* g) {
        Board& board = g->getBoard();
        bool passedGo = false;
        int steps = board.getStepsToNearestRailroad(p->getPosition());
        int target = board.calculateNewPosition(p->getPosition(), steps, passedGo);
        p->moveTo(target);
        if (passedGo) {
            g->awardGoSalary(*p);
        }
        g->handleLanding(*p);
    }
) {}
GoToJailCard::GoToJailCard() : ChanceCard(
    "Masuk Penjara.",
    [](Player* p, Game* g) {
        if (p->isShieldActive()) {
            g->getLogger().log(
                g->getTurnManager().getCurrentTurn(),
                p->getUsername(),
                "SHIELD",
                "Terlindung dari penjara"
            );
            return;
        }
        Board& board = g->getBoard();
        int jailIndex = board.getJailIndex();
        p->moveTo(jailIndex);
        p->enterJail();
    }
) {}
