#include "../../include/core/AccountManager.hpp"

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

bool AccountManager::isUsernameTaken(const string& name) const {
    return accounts.find(name) != accounts.end();
}