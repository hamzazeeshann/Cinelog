#pragma once

#include <cstring>
#include <ctime>

using namespace std;

struct FilmLog {
    int logId;
    char username[32];
    char movieTitle[64];
    float rating;
    long timestamp;

    FilmLog() : logId(0), rating(0.0f), timestamp(0) {
        memset(username, 0, sizeof(username));
        memset(movieTitle, 0, sizeof(movieTitle));
    }

    FilmLog(int id, const char* user, const char* movie, float r) 
        : logId(id), rating(r), timestamp(time(nullptr)) {
        strncpy(username, user, sizeof(username) - 1);
        username[sizeof(username) - 1] = '\0';
        strncpy(movieTitle, movie, sizeof(movieTitle) - 1);
        movieTitle[sizeof(movieTitle) - 1] = '\0';
    }

    void serialize(char* buffer) const {
        memcpy(buffer, this, sizeof(FilmLog));
    }

    void deserialize(const char* buffer) {
        memcpy(this, buffer, sizeof(FilmLog));
    }

    static size_t getSerializedSize() {
        return sizeof(FilmLog);
    }
};
