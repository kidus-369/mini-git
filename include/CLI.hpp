#pragma once

#include <iostream>
#include <string>
#include <filesystem>

#include"CommitHandler.hpp"
#include"FileSystem.hpp"
#include"Utils.hpp"
#include "DatabaseManager.hpp"
using namespace std;

namespace CLI {
    inline const std::string DB_PATH = ".mgit/minigit.db";

    inline bool isRepoInitialized() {
        return FileSystem::folder_already_exists(FileSystem::REPO_FOLDER) && std::filesystem::exists(DB_PATH);
    }

    void printHelp(){
        cout<<"\n minigit- A Simple Version Control System \n";
        cout<<"Commands:\n";
        cout<<"minigit init     -initialize a new repository\n";
        cout<<"minigit commit -m \"msg\" [--author \"name\"] -save a snapshot of your files\n";
        cout<<"minigit log - view commit history in terminal\n";
         cout<<"minigit log --save file - save commit history to a text file\n";
        cout<<"minigit restore <id>    -restore files from a commit\n"; 
        cout<<"minigit help     -show this menu\n";
        cout<<"\n";
    }
    void run(int argc,char*argv[]){
        if(argc < 2){
            cout<<"No command provided.\n";
            printHelp();
            return;
        }
         string command = argv[1];

         if(command =="init"){
            FileSystem::initialize_folders();

                DatabaseManager db(DB_PATH);
                if (db.getHandle() == nullptr) {
                cout << "Error: could not open database.\n";
                return;
            }

                CommitHandler::handleInit(db);
            cout<<"intialized empty minigit repository.\n";
         }
         else if(command == "commit"){

            if (!isRepoInitialized()) {
                cout << "Error: repository not initialized. Run 'minigit init' first.\n";
                return;
            }

            DatabaseManager db(DB_PATH);
            if (db.getHandle() == nullptr) {
                cout << "Error: could not open database.\n";
                return;
            }

            string message;
            string author;
            bool hasMessage = false;

            for (int index = 2; index < argc; ++index) {
                string flag = argv[index];

                if (flag == "-m") {
                    if (index + 1 >= argc) {
                        cout << "Error: commit requires a messsage. \n";
                        cout << "How to use: minigit commit -m\"your message\" [--author \"name\"]\n";
                        return;
                    }

                    message = argv[++index];
                    hasMessage = true;
                } else if (flag == "--author") {
                    if (index + 1 >= argc) {
                        cout << "Error: --author requires a value.\n";
                        cout << "How to use: minigit commit -m\"your message\" [--author \"name\"]\n";
                        return;
                    }

                    author = argv[++index];
                } else {
                    cout << "Error unknown flag" << flag << endl;
                    cout << "How to use: minigit commit -m\"your message\" [--author \"name\"]\n";
                    return;
                }
            }

            if (!hasMessage) {
                cout << "Error: commit requires a messsage. \n";
                cout << "How to use: minigit commit -m\"your message\" [--author \"name\"]\n";
                return;
            }

        string commitID = Utils::generate_alphanumeric(8);
        FileSystem::snapshot(commitID);      
        CommitHandler::Commit commit;
        commit.id        = commitID;
        commit.message   = message;
        commit.author    = author;
        commit.timestamp = Utils::get_db_iso_string(); 

        CommitHandler::CommitResponse result = CommitHandler::handleCommit(db, commit);

            if(result.success){
                cout<<"success your work has been saved to the database. Commit ID: "<<commitID<<"\n";
            }else{
                cout<<"Error: "<<result.message<<"\n";
            }
         }else if(command=="log"){

          if (!isRepoInitialized()) {
              cout << "Error: repository not initialized. Run 'minigit init' first.\n";
              return;
          }

          DatabaseManager db(DB_PATH);
          if (db.getHandle() == nullptr) {
              cout << "Error: could not open database.\n";
              return;
          }
           
          if (argc == 2) {
              CommitHandler::handleLog(db);
          } else if (argc == 4 && string(argv[2]) == "--save") {
              vector<CommitHandler::Commit> commits = CommitHandler::fetchCommits(db, CommitHandler::QueryBuilder::selectAllCommits());

              string logContent;
              for (const auto& commit : commits) {
                  logContent += commit.id + " | " + commit.parent_id + " | " + commit.author + " | " + commit.timestamp + " | " + commit.message + "\n";
              }

              if (FileSystem::write_text_file(argv[3], logContent)) {
                  cout << "Log saved to " << argv[3] << "\n";
              } else {
                  cout << "Error: could not open file for writing.\n";
              }
          } else {
              cout << "Error: invalid log usage.\n";
              cout << "How to use: minigit log [--save file]\n";
          }
       
        }else if(command=="restore"){ 

            if (!isRepoInitialized()) {
                cout << "Error: repository not initialized. Run 'minigit init' first.\n";
                return;
            }

            DatabaseManager db(DB_PATH);
            if (db.getHandle() == nullptr) {
                cout << "Error: could not open database.\n";
                return;
            }

            if(argc < 3){
                cout<<"Error: restore requires a commit ID.\n";
                cout<<"How to use: minigit restore <commitID>\n";
                return;
            } 
        string commitID = argv[2];

        CommitHandler::Commit found;
            if(!CommitHandler::fetchCommitById(db, commitID, found)){
                cout<<"Error: commit ID '"<<commitID<<"' not found.\n";
                return;
            }
        
        FileSystem::restore(commitID);
            cout<<"Restored files from commit: "<<commitID<<"\n";

        
        }else if(command == "help"){ 
        printHelp();

       }else{
        cout<<"Unknown Command!"<<command<<"\n";
        printHelp();
       } 
    
    }
   
}