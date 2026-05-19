#include "Auth.h"
#include <sqlite3.h>

std::optional<User> Auth::login(Database& db, const std::string& username, const std::string& password) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"SQL(
        SELECT u.id, u.login, u.role, COALESCE(d.id, 0)
        FROM Users u
        LEFT JOIN Drivers d ON d.user_id = u.id
        WHERE u.login = ? AND u.password = ?
    )SQL";
    if (sqlite3_prepare_v2(db.raw(), sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    std::optional<User> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = User{sqlite3_column_int(stmt,0),
                      reinterpret_cast<const char*>(sqlite3_column_text(stmt,1)),
                      reinterpret_cast<const char*>(sqlite3_column_text(stmt,2)),
                      sqlite3_column_int(stmt,3)};
    }
    sqlite3_finalize(stmt);
    return result;
}
