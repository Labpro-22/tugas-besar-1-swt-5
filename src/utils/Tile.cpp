#include "utils/Tile.hpp"

Tile::Tile(int id, std::string code, std::string name)
    : id(id), code(code), name(name) {}

Tile::~Tile() {}

int Tile::getId() const { return id; }
std::string Tile::getCode() const { return code; }
std::string Tile::getName() const { return name; }

bool Tile::isOwnable() const { return false; }