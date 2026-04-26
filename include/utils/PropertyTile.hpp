#ifndef PROPERTY_TILE_HPP
#define PROPERTY_TILE_HPP

#include "Tile.hpp"

class Player; // Nanti include kalau udah ada
class Game; // Sama kayak di atas

enum PropertyStatus { 
    BANK, 
    OWNED, 
    MORTGAGED 
};

class PropertyTile : public Tile {
protected:
    Player* owner;
    PropertyStatus status;
    int mortgageValue;

    int festivalMultiplier;
    int festivalDuration;

public:
    PropertyTile(int id, std::string code, std::string name, int mortgageValue);

    virtual int calculateRent(Player* visitor, Game* game) = 0;
    virtual int getLandPrice() const = 0;

    void onLand(Player* player, Game* game) override;

    void setOwner(Player* owner);
    Player* getOwner() const;
    PropertyStatus getStatus() const;
    int getMortgageValue() const;

    int mortgage();
    int redeem();

    bool isOwned() const;
    bool isMortgaged() const;

    void applyFestival();
    void decrementFestivalDuration();
    void resetFestivalIfExpired();
    int getFestivalMultiplier() const;
    int getFestivalDuration() const;

    virtual std::string toString() const;
    virtual bool canBeMortgaged(Game* game) const;

    bool isOwnable() const override;
    virtual void cetakAkta() const;
};

#endif