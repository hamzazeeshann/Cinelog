#include "../include/core/CinelogDB.h"
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
    cout << "Commands: add | get | top | update | delete | search | rating | deleteall | save | exit" << endl;

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

        } else if (command == "search") {
            string substring;
            getline(iss, substring);
            
            if (!substring.empty() && substring[0] == ' ') {
                substring = substring.substr(1);
            }
            
            if (substring.empty()) {
                cout << "Usage: search <substring>" << endl;
                continue;
            }
            
            vector<FilmLog> results = db.searchByTitle(substring.c_str());
            
            if (results.empty()) {
                cout << "No movies found matching: " << substring << endl;
            } else {
                cout << "Found " << results.size() << " movie(s):" << endl;
                for (const auto& log : results) {
                    printLog(log);
                }
            }

        } else if (command == "rating") {
            float minRating, maxRating;
            iss >> minRating >> maxRating;
            
            if (minRating < 0 || maxRating > 5 || minRating > maxRating) {
                cout << "Usage: rating <min> <max> (0-5)" << endl;
                continue;
            }
            
            vector<FilmLog> results = db.getLogsByRating(minRating, maxRating);
            
            if (results.empty()) {
                cout << "No logs found in rating range " << minRating << "-" << maxRating << endl;
            } else {
                cout << "Found " << results.size() << " log(s):" << endl;
                for (const auto& log : results) {
                    printLog(log);
                }
            }

        } else if (command == "deleteall") {
            cout << "Are you sure? This will delete all data (y/n): ";
            string confirm;
            getline(cin, confirm);
            
            if (confirm == "y" || confirm == "Y") {
                db.deleteAll();
                cout << "All data deleted. Database reset." << endl;
            } else {
                cout << "Operation cancelled." << endl;
            }

        } else if (command == "save") {
            db.save();
            cout << "Data persisted to disk." << endl;

        } else if (command == "exit") {
            break;

        } else {
            cout << "Unknown command. Available: add, get, top, update, delete, search, rating, deleteall, save, exit" << endl;
        }
    }

    return 0;
}
