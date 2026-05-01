#pragma once

#include <map>
#include <string>
#include <vector>
#include <cstddef>

#include "Account.hpp"

using namespace std;

class AccountDataManager;
class AccountManager
{
private:
    map<string, Account> accounts;

public:
    // Constructor & Destructor
    AccountManager() = default;
    ~AccountManager() = default;

    AccountManager(const AccountManager &) = delete;
    AccountManager &operator=(const AccountManager &) = delete;

    // Core Methods
    void addAccount(const Account &newAccount);
    Account *getAccount(const string &name, const string &pass);
    vector<Account> getTopAccounts(size_t limit) const;

    bool isUsernameTaken(const string &name) const;

    friend AccountDataManager;
};
