#include "../../include/core/Account.hpp"

using namespace std;

Account::Account(const string& uname, const string& pass, int initialScore)
    : username(uname), password(pass), score(initialScore) {}

// Core Methods
void Account::addScore(int num) {
    score += num;
}

// Operator Overloading
bool Account::operator<(const Account& other) const {
    return this->score < other.score;
}

bool Account::operator>(const Account& other) const {
    return this->score > other.score;
}

bool Account::operator==(const Account& other) const {
    return this->username == other.username;
}

// Getters & Security
string Account::getUsername() const {
    return username;
}

int Account::getScore() const {
    return score;
}

bool Account::validatePassword(const string& inputPassword) const {
    return this->password == inputPassword;
}