#ifndef COMMIT_HANDLER_HPP
#define COMMIT_HANDLER_HPP

#include <string>
#include <iostream>
#include <vector>

extern "C" {
    #include "sqlite3.h"
}

#include "DatabaseManager.hpp"
#include "FileSystem.hpp"
#include "Utils.hpp"




namespace CommitHandler {

    class QueryBuilder {
    public:
        static std::string selectAllCommits() {
            return "SELECT id, parent_id, message, author, date FROM commits ORDER BY date DESC;";
        }

        static std::string selectCommitById(const std::string& id) {
            return "SELECT id, parent_id, message, author, date FROM commits WHERE id = '" + id + "';";
        }

        static std::string selectHistoryFrom(const std::string& startId) {
            return "SELECT id, parent_id, message, author, date FROM commits WHERE id = '" + startId + "';";
        }
    };
    
    struct CommitResponse {
        bool success;
        std::string message;
    };

    struct Commit {
        std::string id;
        std::string parent_id; 
        std::string message;
        std::string timestamp;
        std::string author; 
    };

    inline Commit buildCommitFromStatement(sqlite3_stmt* stmt) {
        Commit commit;

        const unsigned char* id = sqlite3_column_text(stmt, 0);
        const unsigned char* parent = sqlite3_column_text(stmt, 1);
        const unsigned char* msg = sqlite3_column_text(stmt, 2);
        const unsigned char* auth = sqlite3_column_text(stmt, 3);
        const unsigned char* date = sqlite3_column_text(stmt, 4);

        if (id) commit.id = reinterpret_cast<const char*>(id);
        if (parent) commit.parent_id = reinterpret_cast<const char*>(parent);
        if (msg) commit.message = reinterpret_cast<const char*>(msg);
        if (auth) commit.author = reinterpret_cast<const char*>(auth);
        if (date) commit.timestamp = reinterpret_cast<const char*>(date);

        return commit;
    }

    std::vector<Commit> fetchCommits(DatabaseManager& db, const std::string& query) {
        std::vector<Commit> results;
        sqlite3* handle = db.getHandle();
        if (handle == nullptr) {
            return results;
        }

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(handle, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            return results;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            results.push_back(buildCommitFromStatement(stmt));
        }

        sqlite3_finalize(stmt);
        return results;
    }

    bool fetchCommitById(DatabaseManager& db, const std::string& id, Commit& commit) {
        std::vector<Commit> commits = fetchCommits(db, QueryBuilder::selectCommitById(id));
        if (commits.empty()) {
            return false;
        }

        commit = commits.front();
        return true;
    }

    bool fetchSingleText(DatabaseManager& db, const std::string& query, std::string& value) {
        sqlite3* handle = db.getHandle();
        if (handle == nullptr) {
            return false;
        }

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(handle, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        bool found = false;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* text = sqlite3_column_text(stmt, 0);
            if (text != nullptr) {
                value = reinterpret_cast<const char*>(text);
                found = true;
            }
        }

        sqlite3_finalize(stmt);
        return found;
    }

    void handleInit(DatabaseManager& db) {

        std::string commit_table = "CREATE TABLE IF NOT EXISTS commits ("
                            "id TEXT PRIMARY KEY, "
                            "parent_id TEXT, "
                            "message TEXT, "
                            "author VARCHAR, "
                            "date TEXT);";
                              
        std::string metadata_table = "CREATE TABLE IF NOT EXISTS metadata ("
                            "key TEXT PRIMARY KEY, "
                            "value TEXT);";
        if (db.executeQuery(commit_table) && db.executeQuery(metadata_table)) {
            db.executeQuery("INSERT OR IGNORE INTO metadata (key, value) VALUES ('HEAD', 'none');");
            std::cout << "Success: MiniGit repository initialized." << std::endl;
        } else {
            std::cerr << "Error: Database table creation failed." << std::endl;
        }

    }
    CommitResponse handleCommit(DatabaseManager& db, Commit commit) {
        if (commit.id.empty()) {
            return {false, "Missing critical field: Commit ID."};
        }
        if (commit.message.empty()) {
            return {false, "Commit message cannot be empty."};
        }

        std::string finalAuthor = commit.author.empty() ? "Unknown Author" : commit.author;
        std::string finalParent = commit.parent_id;
        if (finalParent.empty()) {
            std::string currentHead;
            if (!fetchSingleText(db, "SELECT value FROM metadata WHERE key = 'HEAD';", currentHead) || currentHead == "none") {
                finalParent = "none";
            } else {
                finalParent = currentHead;
            }
        }

      
        std::string sql = "INSERT INTO commits (id, parent_id, message, author, date) VALUES ("
                          "'" + commit.id + "', "
                          "'" + finalParent + "', "
                          "'" + commit.message + "', "
                          "'" + finalAuthor + "', "
                          "'" + commit.timestamp + "');";

        std::string updateHead = "UPDATE metadata SET value = '" + commit.id + "' WHERE key = 'HEAD';";
        
        if (!db.executeQuery("BEGIN TRANSACTION;")) {
            return {false, "Database Error: Could not start transaction."};
        }

        if (db.executeQuery(sql) && db.executeQuery(updateHead)) {
            if (db.executeQuery("COMMIT;")) {
                return {true, "Commit " + commit.id + " finalized successfully."};
            }
        }

        // If it reaches here, something went wrong
        db.executeQuery("ROLLBACK;");
        return {false, "Database Error: Atomic update failed. Changes rolled back."};
    }
    
    CommitResponse handleReverse(DatabaseManager& db, const std::string& commitID) {
        Commit commit;
        if (!fetchCommitById(db, commitID, commit)) {
            return {false, "Commit not found: " + commitID + "."};
        }

        std::string nextHead = commit.parent_id.empty() ? "none" : commit.parent_id;
        std::string deleteCommitSql = "DELETE FROM commits WHERE id = '" + commitID + "';";
        std::string updateHeadSql = "UPDATE metadata SET value = '" + nextHead + "' WHERE key = 'HEAD';";

        if (!db.executeQuery("BEGIN TRANSACTION;")) {
            return {false, "Database Error: Could not start transaction."};
        }

        if (db.executeQuery(deleteCommitSql) && db.executeQuery(updateHeadSql)) {
            if (db.executeQuery("COMMIT;")) {
                return {true, "Commit " + commitID + " reversed successfully."};
            }
        }

        db.executeQuery("ROLLBACK;");
        return {false, "Database Error: Reverse operation failed. Changes rolled back."};
    }
    
    void handleLog(DatabaseManager& db) {
            std::vector<Commit> commits = fetchCommits(db, QueryBuilder::selectAllCommits());
        for (const auto& commit : commits) {
            std::cout << commit.id << " | " << commit.parent_id << " | " << commit.author << " | " << commit.timestamp << " | " << commit.message << std::endl;
        }
    }

}

#endif