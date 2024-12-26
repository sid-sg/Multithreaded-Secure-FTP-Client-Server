#ifndef DB_HPP
#define DB_HPP

#include <sqlite3.h>

#include <string>

class Database {
   private:
    sqlite3* db;

   public:
    explicit Database(const std::string& dbPath);
    ~Database();
    bool initTable();
    bool executeQuery(const std::string& query);
    bool registerUser(const std::string& username, const std::string& plainPassword);
    bool userExists(const std::string& username);
    std::string getHashedPassword(const std::string& username);
};

#endif
