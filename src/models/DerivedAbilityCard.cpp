#include "../../include/models/DerivedAbilityCard.hpp"

std::string AbilityCard::escapeSerializedValue(const std::string& value)
{
    std::string result;
    result.reserve(value.size());

    for (char c : value)
    {
        switch (c)
        {
            case '\\': result += "\\\\"; break;
            case '|':  result += "\\|"; break;
            case '=':  result += "\\="; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:   result += c; break;
        }
    }

    return result;
}

std::string AbilityCard::serializeCommonAbilityFields(
    const std::string& name,
    const std::string& description
)
{
    return "NAME=" + escapeSerializedValue(name) +
           "|DESCRIPTION=" + escapeSerializedValue(description);
}

bool AbilityCard::canUse(Player* target, Game* game) const
{
    (void) target;
    (void) game;
    return true;
}

std::string AbilityCard::serialize() const
{
    return serializeCommonAbilityFields(this->name, this->description);
}

MoveCard::MoveCard() : AbilityCard("MoveCard", "Maju x petak."), steps(0) {}

MoveCard::MoveCard(int steps) : AbilityCard("MoveCard", "Maju " + std::to_string(steps) + " petak."), steps(steps) {}
void MoveCard::setup() {
    std::mt19937 randomizer(std::random_device{}());
    std::uniform_int_distribution picker(1, 12);
    steps = picker(randomizer);
    description = "Maju " + std::to_string(steps) + " petak.";
}
void MoveCard::use(Player* target, Game* game) {
    // TODO: MOVE PLAYER
}

std::string MoveCard::serialize() const 
{
    return serializeCommonAbilityFields(this->name, this->description) + "|STEPS=" + std::to_string(this->steps);
}

DiscountCard::DiscountCard() : AbilityCard("DiscountCard", "Dapatkan diskon selama 1 giliran."), percentage(0) {}

DiscountCard::DiscountCard(int percentage)
    : AbilityCard("DiscountCard", "Dapatkan diskon " + std::to_string(percentage) + "%% selama 1 giliran."), percentage(percentage) {}
void DiscountCard::setup() {
    std::mt19937 randomizer(std::random_device{}());
    std::uniform_int_distribution picker(1, 100);
    percentage = picker(randomizer);
    description = "Dapatkan diskon " + std::to_string(percentage) + "%% selama 1 giliran.";
}
void DiscountCard::use(Player* target, Game* game) {
    // TODO: APPLY DISCOUNT
}

std::string DiscountCard::serialize() const
{
    return serializeCommonAbilityFields(this->name, this-> description) + "|PERCENTAGE=" + std::to_string(this->percentage);
}

ShieldCard::ShieldCard() : AbilityCard("ShieldCard", "Terlindung dari tagihan sewa maupun sanksi apapun yang merugikan selama 1 giliran.") {}
void ShieldCard::use(Player* target, Game* game) {
    // TODO: APPLY SHIELD
}

std::string ShieldCard::serialize() const
{
    return serializeCommonAbilityFields(this->name, this-> description);
}

TeleportCard::TeleportCard() : AbilityCard("TeleportCard", "Bebas berpindah ke petak manapun di atas papan permainan.") {}

void TeleportCard::use(Player* target, Game* game) {
    // TODO: TELEPORT
}

std::string TeleportCard::serialize() const
{
    return serializeCommonAbilityFields(this->name, this-> description);
}


LassoCard::LassoCard() : AbilityCard("LassoCard", "Tarik satu pemain lain di depanmu ke posisimu.") {}
void LassoCard::use(Player* target, Game* game) {
    // TODO: PULL OTHER PLAYER
}

std::string LassoCard::serialize() const
{
    return serializeCommonAbilityFields(this->name, this-> description);
}

DemolitionCard::DemolitionCard() : AbilityCard("DemolitionCard", "Hancurkan satu properti milik pemain lain.") {}
void DemolitionCard::use(Player* target, Game* game) {
    // TODO: DEMOLISH
}

std::string DemolitionCard::serialize() const
{
    return serializeCommonAbilityFields(this->name, this-> description);
}