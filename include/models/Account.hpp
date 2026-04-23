#pragma once
#include <string>

class Account
{
private:
    std::string username;
    std::string password;
    int score;
public:
    Account(std::string username, std::string password, int score) : username(username), password(password), score(score) {}
    Account(std::string username, std::string password) : username(username), password(password), score(0) {}
    std::string getName() const {
        return username;
    }
    int getScore() const {
        return score;
    }
    bool isPasswordMatch(std::string input) const {
        return input == password;
    }
    void addScore(int num) {
        score += num;
    }
    bool operator<(const Account& other) const {
        if (this->score != other.score) return this->score < other.score;
        return this->username < other.username;
    }
    bool operator>(const Account& other) const {
        if (this->score != other.score) return this->score > other.score;
        return this->username > other.username;
    }
    bool operator==(const Account& other) const {
        return this->score == other.score;
    }
};