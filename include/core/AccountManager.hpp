#pragma once

#include <map>
#include <string>

#include "Account.hpp" 

using namespace std;

class AccountManager {
private:
    map<string, Account> accounts;

public:
    // Constructor & Destructor
    AccountManager() = default;
    ~AccountManager() = default;

    AccountManager(const AccountManager&) = delete;
    AccountManager& operator=(const AccountManager&) = delete;

    // Core Methods
    void addAccount(const Account& newAccount);
    Account* getAccount(const string& name, const string& pass);
    
    bool isUsernameTaken(const string& name) const;
};