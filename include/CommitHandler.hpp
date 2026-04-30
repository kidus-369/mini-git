#ifndef COMMIT_HANDLER_HPP
#define COMMIT_HANDLER_HPP

#include <string>
#include <iostream>

#include "DatabaseManager.hpp"
#include "FileSystem.hpp"
#include "Utils.hpp"

namespace CommitHandler {
    
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

      
        std::string sql = "INSERT INTO commits (id, message, author, date) VALUES ("
                          "'" + commit.id + "', "
                          "'" + commit.message + "', "
                          "'" + finalAuthor + "', "
                          "'" + commit.timestamp + "');";

        std::string updateHead = "UPDATE metadata SET value = '" + commit.id + "' WHERE key = 'HEAD';";
        
        if (!db.executeQuery("BEGIN TRANSACTION;")) {
            return {false, "Database Error: Could not start transaction."};
        }

        if (db.executeQuery(insertSql) && db.executeQuery(updateHead)) {
            if (db.executeQuery("COMMIT;")) {
                return {true, "Commit " + commit.id + " finalized successfully."};
            }
        }

        // If it reaches here, something went wrong
        db.executeQuery("ROLLBACK;");
        return {false, "Database Error: Atomic update failed. Changes rolled back."};
    }
    
    void handleReverse(DatabaseManager& db, const std::string& commitID);
    
    void handleLog(DatabaseManager& db);

}

#endif