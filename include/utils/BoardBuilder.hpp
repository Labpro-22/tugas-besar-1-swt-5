#ifndef BOARD_BUILDER_HPP
#define BOARD_BUILDER_HPP

#include <string>
#include <vector>
#include <tuple>
#include <map>
#include "Board.hpp"

// BoardBuilder: membangun Board dari data config mentah.
// File ini TIDAK include Config.hpp untuk menghindari naming conflict PPH/PBM.
class BoardBuilder {
public:
    static void build(
        Board& board,
        const std::vector<std::tuple<int,std::string,std::string,std::string,std::string,int,int,std::vector<int>>>& propConfigs,
        const std::vector<std::tuple<int,std::string,std::string,std::string,std::string>>& actionConfigs,
        const std::map<int,int>& railroadRent,
        const std::map<int,int>& utilityMult
    );
};

#endif
