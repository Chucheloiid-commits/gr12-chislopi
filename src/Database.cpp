#include "Database.h"
#include <stdexcept>

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        std::string msg = sqlite3_errmsg(db);
        sqlite3_close(db);
        throw std::runtime_error("SQLite open error: " + msg);
    }
    exec("PRAGMA foreign_keys = ON;");
}

Database::~Database() {
    if (db) sqlite3_close(db);
}

void Database::exec(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw std::runtime_error(msg);
    }
}

std::vector<std::vector<std::string>> Database::query(const std::string& sql) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db));
    std::vector<std::vector<std::string>> rows;
    int cols = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::vector<std::string> row;
        for (int i = 0; i < cols; ++i) {
            const unsigned char* text = sqlite3_column_text(stmt, i);
            row.emplace_back(text ? reinterpret_cast<const char*>(text) : "NULL");
        }
        rows.push_back(row);
    }
    sqlite3_finalize(stmt);
    return rows;
}
