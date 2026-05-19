#pragma once
#include "AutoparkService.h"
#include "Auth.h"

class ConsoleUI {
    Database& db;
    AutoparkService service;
public:
    explicit ConsoleUI(Database& database) : db(database), service(database) {}
    void run();
private:
    void adminMenu(const User& user);
    void driverMenu(const User& user);
};
