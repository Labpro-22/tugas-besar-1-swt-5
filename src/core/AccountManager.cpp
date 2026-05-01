#include "../../include/core/AccountManager.hpp"
#include <algorithm>

using namespace std;

void AccountManager::addAccount(const Account& newAccount) {
    accounts[newAccount.getUsername()] = newAccount;
}

Account* AccountManager::getAccount(const string& name, const string& pass) {
    auto it = accounts.find(name);
    if (it != accounts.end()) {
        if (it->second.validatePassword(pass)) {
            return &(it->second);
        }
    }
    return nullptr;
}

vector<Account> AccountManager::getTopAccounts(size_t limit) const {
    vector<Account> result;
    result.reserve(accounts.size());

    for (const auto& pair : accounts) {
        result.push_back(pair.second);
    }

    sort(result.begin(), result.end(), [](const Account& a, const Account& b) {
        return a > b;
    });

    if (result.size() > limit) {
        result.resize(limit);
    }

    return result;
}

bool AccountManager::isUsernameTaken(const string& name) const {
    return accounts.find(name) != accounts.end();
}
