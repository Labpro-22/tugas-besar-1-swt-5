#ifndef TILE_HPP
#define TILE_HPP

#include <string>
class Player; // Nanti diinclude
class Game; // Nanti diinclude

class Tile {
protected:
    int id;
    std::string code;
    std::string name;

public:
    Tile(int id, std::string code, std::string name);
    virtual ~Tile();

    virtual void onLand(Player* player, Game* game) = 0;

    int getId() const;
    std::string getCode() const;
    std::string getName() const;

    virtual bool isOwnable() const;
};

#endif