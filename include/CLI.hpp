#pragma once

#include <iostream>
#include <string>

#include"CommitHandler.hpp"
#include"FileSystem.hpp"

using namespace std;

namespace CLI {
    void printHelp(){
        cout<<"\n minigit- A Simple Version Control System \n";
        cout<<"Commands:\n";
        cout<<"minigit init     -initialize a new repository\n";
        cout<<"minigit commit -m\"msg\" -save a snapshot of your files\n";
        cout<<"minigit log      -view commit history\n";
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
            FileSystem::init();
            cout<<"intialized empty minigit repository.\n";
         }
         else if(command == "commit"){

            if(argc < 4){
                cout<<"Error: commit requires a messsage. \n";
                cout<<"How to use: minigit commit -m\"your message\"\n";
                return;
            }   
        string flag  = argv[2];
        if (flag!="-m"){
            cout<<"Error unknown flag"<<flag<<endl;
            cout<<"How to use: minigit commit -m\"your message\"n";
            return;
        }

        string message = argv[3];
        CommitHandler::commit(message);
        
       }else if(command=="log"){
        CommitHandler::log();
       
       }else if(command=="Help"){
        printHelp();

       }else{
        cout<<"Unknown Command!"<<command<<"\n";
        printHelp();
       } 
    
    }
   
}