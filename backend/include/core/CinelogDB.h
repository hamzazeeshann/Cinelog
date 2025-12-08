#pragma once

#include "../ds/BTree.h"
#include "../ds/HashMap.h"
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;

class CinelogDB {
private:
    BTree* btree;
    HashMap<const char*, vector<int>>* hashIndex;
    int nextLogId;
    string dbFilename;

    char* copyString(const char* str) {
        size_t len = strlen(str);
        char* copy = new char[len + 1];
        strcpy(copy, str);
        return copy;
    }

public:
    CinelogDB(const string& dbFile) : nextLogId(1), dbFilename(dbFile) {
        btree = new BTree(dbFile);
        hashIndex = new HashMap<const char*, vector<int>>(32);
        rebuildHashIndex();
        nextLogId = btree->getMaxLogId() + 1;
    }

    ~CinelogDB() {
        delete btree;
        delete hashIndex;
    }

    void rebuildHashIndex() {
        hashIndex->clear();
        vector<FilmLog> allRecords = btree->getAllRecords();
        for (const auto& log : allRecords) {
            vector<int>* userLogs = hashIndex->get(log.username);
            if (userLogs == nullptr) {
                vector<int> newList;
                newList.push_back(log.logId);
                hashIndex->insert(copyString(log.username), newList);
            } else {
                userLogs->push_back(log.logId);
            }
        }
    }

    int addLog(const char* username, const char* movieTitle, float rating) {
        FilmLog log(nextLogId, username, movieTitle, rating);
        btree->insert(log);
        
        vector<int>* userLogs = hashIndex->get(username);
        if (userLogs == nullptr) {
            vector<int> newList;
            newList.push_back(nextLogId);
            hashIndex->insert(copyString(username), newList);
        } else {
            userLogs->push_back(nextLogId);
        }
        
        return nextLogId++;
    }

    bool getLog(int logId, FilmLog& result) {
        return btree->search(logId, result);
    }

    bool updateLog(int logId, const char* movieTitle, float rating) {
        FilmLog existingLog;
        if (!btree->search(logId, existingLog)) {
            return false;
        }

        FilmLog updatedLog = existingLog;
        if (movieTitle != nullptr && strlen(movieTitle) > 0) {
            strncpy(updatedLog.movieTitle, movieTitle, sizeof(updatedLog.movieTitle) - 1);
            updatedLog.movieTitle[sizeof(updatedLog.movieTitle) - 1] = '\0';
        }
        if (rating >= 0 && rating <= 5) {
            updatedLog.rating = rating;
        }
        updatedLog.timestamp = time(nullptr);

        return btree->updateRecord(logId, updatedLog);
    }

    bool deleteLog(int logId) {
        FilmLog log;
        if (!btree->search(logId, log)) {
            return false;
        }
        
        bool deleted = btree->deleteRecord(logId);
        if (deleted) {
            vector<int>* userLogs = hashIndex->get(log.username);
            if (userLogs != nullptr) {
                for (size_t i = 0; i < userLogs->size(); i++) {
                    if ((*userLogs)[i] == logId) {
                        userLogs->erase(userLogs->begin() + i);
                        break;
                    }
                }
            }
        }
        return deleted;
    }

    void deleteAll() {
        delete btree;
        delete hashIndex;
        
        remove(dbFilename.c_str());
        
        btree = new BTree(dbFilename);
        hashIndex = new HashMap<const char*, vector<int>>(32);
        nextLogId = 1;
    }

    vector<FilmLog> searchByTitle(const char* substring) {
        vector<FilmLog> results;
        vector<FilmLog> allRecords = btree->getAllRecords();
        
        for (const auto& log : allRecords) {
            if (strstr(log.movieTitle, substring) != nullptr) {
                results.push_back(log);
            }
        }
        
        return results;
    }

    vector<FilmLog> getLogsByRating(float minRating, float maxRating) {
        vector<FilmLog> results;
        vector<FilmLog> allRecords = btree->getAllRecords();
        
        for (const auto& log : allRecords) {
            if (log.rating >= minRating && log.rating <= maxRating) {
                results.push_back(log);
            }
        }
        
        return results;
    }

    vector<FilmLog> getUserTopLogs(const char* username, int k) {
        vector<int>* logIds = hashIndex->get(username);
        vector<FilmLog> logs;

        if (logIds != nullptr) {
            for (int id : *logIds) {
                FilmLog log;
                if (btree->search(id, log)) {
                    logs.push_back(log);
                }
            }
        }

        sort(logs.begin(), logs.end(), [](const FilmLog& a, const FilmLog& b) {
            return a.rating > b.rating;
        });

        if (logs.size() > static_cast<size_t>(k)) {
            logs.resize(k);
        }

        return logs;
    }

    void save() {
    }
};
