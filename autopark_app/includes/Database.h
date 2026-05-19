#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <functional>

class Database {
    sqlite3* db{};
public:
    explicit Database(const std::string& path);
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    void exec(const std::string& sql);
    std::vector<std::vector<std::string>> query(const std::string& sql);
    sqlite3* raw() { return db; }
};
