#pragma once

#include "bTree.h"
#include "userHashIndex.h"
#include <vector>
#include <algorithm>

using namespace std;

class CinelogDB {
private:
    BTree* btree;
    UserHashIndex hashIndex;
    int nextLogId;

public:
    CinelogDB(const string& dbFile) : nextLogId(1) {
        btree = new BTree(dbFile);
        rebuildHashIndex();
        nextLogId = btree->getMaxLogId() + 1;
    }

    ~CinelogDB() {
        delete btree;
    }

    void rebuildHashIndex() {
        hashIndex.clear();
        vector<FilmLog> allRecords = btree->getAllRecords();
        for (const auto& log : allRecords) {
            hashIndex.addEntry(log.username, log.logId);
        }
    }

    int addLog(const char* username, const char* movieTitle, float rating) {
        FilmLog log(nextLogId, username, movieTitle, rating);
        btree->insert(log);
        hashIndex.addEntry(username, nextLogId);
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
            hashIndex.removeEntry(log.username, logId);
        }
        return deleted;
    }

    vector<FilmLog> getUserTopLogs(const char* username, int k) {
        vector<int> logIds = hashIndex.getUserLogs(username);
        vector<FilmLog> logs;

        for (int id : logIds) {
            FilmLog log;
            if (btree->search(id, log)) {
                logs.push_back(log);
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
