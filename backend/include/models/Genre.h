#pragma once

#include <cstring>

using namespace std;

struct Genre {
    int genre_id;
    char name[32];

    Genre() : genre_id(0) {
        memset(name, 0, sizeof(name));
    }

    Genre(int gid, const char* n) : genre_id(gid) {
        strncpy(name, n, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
    }

    void serialize(char* buffer) const {
        memcpy(buffer, this, sizeof(Genre));
    }

    void deserialize(const char* buffer) {
        memcpy(this, buffer, sizeof(Genre));
    }

    static size_t getSerializedSize() {
        return sizeof(Genre);
    }

    int getId() const {
        return genre_id;
    }
};
