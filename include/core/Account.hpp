#pragma once

#include <string>

using namespace std;

class Account {
private:
    string username;
    string password;
    int score;

public:
    // Constructor
    Account() = default; 
    Account(const string& uname, const string& pass, int initialScore = 0);

    // Core Methods
    void addScore(int num);

    // Operator Overloading
    bool operator<(const Account& other) const;
    bool operator>(const Account& other) const;
    bool operator==(const Account& other) const;

    // Getters & Security
    string getUsername() const;
    int getScore() const;
    
    bool validatePassword(const string& inputPassword) const;
};