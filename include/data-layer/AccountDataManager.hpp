#ifndef ACCOUNT_DATA_MANAGER_HPP
#define ACCOUNT_DATA_MANAGER_HPP

#include <fstream>
#include <string>
#include <stdexcept>
#include "../core/Account.hpp"
#include "../core/AccountManager.hpp"

class AccountDataManager
{
private:
    std::fstream accountDataStream;
    std::string filePath;

public:
    AccountDataManager(const std::string &filePath);

    void saveData(const AccountManager &accountManager);
    void loadData(AccountManager &manager);
};

#endif
