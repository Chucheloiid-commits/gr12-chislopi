#pragma once
#include "Database.h"
#include <optional>
#include <string>

struct User {
    int id{};
    std::string login;
    std::string role;
    int driver_id{};
};

class Auth {
public:
    static std::optional<User> login(Database& db, const std::string& username, const std::string& password);
};
