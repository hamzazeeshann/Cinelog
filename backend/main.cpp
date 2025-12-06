#include "cinelogDB.h"
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

void printLog(const FilmLog& log) {
    cout << "ID: " << log.logId 
              << " | User: " << log.username 
              << " | Movie: " << log.movieTitle 
              << " | Rating: " << fixed << setprecision(1) << log.rating 
              << " | Time: " << log.timestamp << endl;
}

int main() {
    CinelogDB db("tree.bin");
    string line, command;

    cout << "Cinelog Database Engine" << endl;
    cout << "Commands: add <user> <movie> <rating> | get <id> | top <user> | update <id> <movie> <rating> | delete <id> | save | exit" << endl;

    while (true) {
        cout << "> ";
        if (!getline(cin, line)) {
            break;
        }

        istringstream iss(line);
        iss >> command;

        if (command == "add") {
            string username, movieTitle;
            float rating;
            
            iss >> username;
            
            string word;
            ostringstream titleBuilder;
            bool firstWord = true;
            
            while (iss >> word) {
                try {
                    rating = stof(word);
                    break;
                } catch (...) {
                    if (!firstWord) {
                        titleBuilder << " ";
                    }
                    titleBuilder << word;
                    firstWord = false;
                }
            }
            
            movieTitle = titleBuilder.str();
            
            if (username.empty() || movieTitle.empty() || rating < 0 || rating > 5) {
                cout << "Invalid input. Usage: add <user> <movie> <rating>" << endl;
                continue;
            }
            
            int logId = db.addLog(username.c_str(), movieTitle.c_str(), rating);
            cout << "Added log ID: " << logId << endl;

        } else if (command == "get") {
            int logId;
            iss >> logId;
            
            FilmLog log;
            if (db.getLog(logId, log)) {
                printLog(log);
            } else {
                cout << "Log not found." << endl;
            }

        } else if (command == "top") {
            string username;
            iss >> username;
            
            if (username.empty()) {
                cout << "Usage: top <user>" << endl;
                continue;
            }
            
            vector<FilmLog> topLogs = db.getUserTopLogs(username.c_str(), 10);
            
            if (topLogs.empty()) {
                cout << "No logs found for user: " << username << endl;
            } else {
                cout << "Top rated movies for " << username << ":" << endl;
                for (const auto& log : topLogs) {
                    printLog(log);
                }
            }

        } else if (command == "update") {
            int logId;
            float rating;
            iss >> logId;
            
            string word;
            ostringstream titleBuilder;
            bool firstWord = true;
            
            while (iss >> word) {
                try {
                    rating = stof(word);
                    break;
                } catch (...) {
                    if (!firstWord) {
                        titleBuilder << " ";
                    }
                    titleBuilder << word;
                    firstWord = false;
                }
            }
            
            string movieTitle = titleBuilder.str();
            
            if (db.updateLog(logId, movieTitle.c_str(), rating)) {
                cout << "Updated log ID: " << logId << endl;
            } else {
                cout << "Log not found." << endl;
            }

        } else if (command == "delete") {
            int logId;
            iss >> logId;
            
            if (db.deleteLog(logId)) {
                cout << "Deleted log ID: " << logId << endl;
            } else {
                cout << "Log not found." << endl;
            }

        } else if (command == "save") {
            db.save();
            cout << "Data persisted to disk." << endl;

        } else if (command == "exit") {
            break;

        } else {
            cout << "Unknown command. Available: add, get, top, update, delete, save, exit" << endl;
        }
    }

    return 0;
}
