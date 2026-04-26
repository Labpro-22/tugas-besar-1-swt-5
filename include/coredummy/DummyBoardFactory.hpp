#ifndef COREDUMMY_DUMMY_BOARD_FACTORY_HPP
#define COREDUMMY_DUMMY_BOARD_FACTORY_HPP

#include <vector>
#include "coredummy/GameViewModel.hpp"

class DummyBoardFactory {
public:
    static std::vector<TileViewData> createDefaultBoard();
};

#endif
