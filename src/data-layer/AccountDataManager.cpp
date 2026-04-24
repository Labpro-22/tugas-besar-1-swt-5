#include "../../include/data-layer/AccountDataManager.hpp"

AccountDataManager::AccountDataManager(const std::string &filePath)
    : filePath(filePath)
{
}

void AccountDataManager::saveData(const AccountManager &accountManager)
{
    accountDataStream.open(filePath, std::ios::out);

    if (!accountDataStream.is_open())
    {
        throw std::runtime_error("Gagal membuka file account untuk write: " + filePath);
    }

    accountDataStream << "=== Account Data ===\n";

    for (const auto &pair : accountManager.accounts)
    {
        const Account &account = pair.second;

        accountDataStream
            << account.username << " "
            << account.password << " "
            << account.score << "\n";
    }

    accountDataStream.close();
}

void AccountDataManager::loadData(AccountManager &manager)
{
    AccountManager accountManager;

    accountDataStream.open(filePath, std::ios::in);

    if (!accountDataStream.is_open())
    {
        throw std::runtime_error("Gagal membuka file account untuk read: " + filePath);
    }

    std::string line;
    std::getline(accountDataStream, line);

    std::string username;
    std::string password;
    int score;

    while (accountDataStream >> username >> password >> score)
    {
        Account account(username, password, score);
        accountManager.addAccount(account);
    }

    accountDataStream.close();
}
