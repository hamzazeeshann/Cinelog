#pragma once

#include <cstring>
#include <ctime>

using namespace std;

struct Log {
    int log_id;
    int user_id;
    int film_id;
    float rating;
    char review_preview[256];
    long watch_date;

    Log() : log_id(0), user_id(0), film_id(0), rating(0.0f), watch_date(0) {
        memset(review_preview, 0, sizeof(review_preview));
    }

    Log(int lid, int uid, int fid, float r, const char* review) 
        : log_id(lid), user_id(uid), film_id(fid), rating(r), watch_date(time(nullptr)) {
        strncpy(review_preview, review, sizeof(review_preview) - 1);
        review_preview[sizeof(review_preview) - 1] = '\0';
    }

    void serialize(char* buffer) const {
        memcpy(buffer, this, sizeof(Log));
    }

    void deserialize(const char* buffer) {
        memcpy(this, buffer, sizeof(Log));
    }

    static size_t getSerializedSize() {
        return sizeof(Log);
    }

    int getId() const {
        return log_id;
    }
};
