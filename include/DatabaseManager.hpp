#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include <iostream>
#include <string>

extern "C" {
    #include "sqlite3.h"
}


class DatabaseManager {
public:
    DatabaseManager(const std::string& dbName) {
        this->DB = nullptr;

        if (sqlite3_open(dbName.c_str(), &this->DB) != SQLITE_OK) {
            std::cerr << "Database connection failed: " << sqlite3_errmsg(this->DB) << std::endl;
            this->DB = nullptr; 
        }
    }

    ~DatabaseManager() {
        if (this->DB != nullptr) {
            sqlite3_close(this->DB);
        }
    }

    bool executeQuery(const std::string& query) {
        if (this->DB == nullptr) return false;
        
        int result = sqlite3_exec(this->DB, query.c_str(), nullptr, nullptr, nullptr);
        return (result == SQLITE_OK);
    }

    sqlite3* getHandle() const {
        return this->DB;
    }

private:
    sqlite3* DB;
};

#endif