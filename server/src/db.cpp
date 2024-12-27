#include "../include/db.hpp"

Database::Database(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Failed to open SQLite database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    } else {
        std::cout << "Database initialized\n";
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
}

bool Database::initTable() {
    std::string createTableQuery =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL UNIQUE, "
        "hashedPassword TEXT NOT NULL);";

    return executeQuery(createTableQuery);
}

bool Database::executeQuery(const std::string& query) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQLite query error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool Database::registerUser(const std::string& username, const std::string& password) {
    sqlite3_stmt* stmt;
    std::string query = "INSERT INTO users (username, hashedPassword) VALUES (?, ?);";

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

bool Database::userExists(const std::string& username) {
    sqlite3_stmt* stmt;
    std::string query = "SELECT 1 FROM users WHERE username = ? LIMIT 1;";

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    bool exists = sqlite3_step(stmt) == SQLITE_ROW;

    sqlite3_finalize(stmt);
    return exists;
}

std::string Database::getHashedPassword(const std::string& username) {
    sqlite3_stmt* stmt;
    std::string query = "SELECT hashedPassword FROM users WHERE username = ? LIMIT 1;";
    std::string hashedPassword;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return "";
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (password) {
            hashedPassword = password;
        }
    }

    sqlite3_finalize(stmt);
    return hashedPassword;
}
