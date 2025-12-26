#pragma once

#include <cstring>

using namespace std;

struct Film {
    int film_id;
    int tmdb_id;
    char title[64];
    int release_year;
    int runtime;
    char cast_summary[256];
    char director[64];
    int genre_ids[3];

    Film() : film_id(0), tmdb_id(0), release_year(0), runtime(0) {
        memset(title, 0, sizeof(title));
        memset(cast_summary, 0, sizeof(cast_summary));
        memset(director, 0, sizeof(director));
        memset(genre_ids, 0, sizeof(genre_ids));
    }

    Film(int fid, int tid, const char* t, int year, int rt, const char* cast, const char* dir) 
        : film_id(fid), tmdb_id(tid), release_year(year), runtime(rt) {
        strncpy(title, t, sizeof(title) - 1);
        title[sizeof(title) - 1] = '\0';
        strncpy(cast_summary, cast, sizeof(cast_summary) - 1);
        cast_summary[sizeof(cast_summary) - 1] = '\0';
        strncpy(director, dir, sizeof(director) - 1);
        director[sizeof(director) - 1] = '\0';
        memset(genre_ids, 0, sizeof(genre_ids));
    }

    void serialize(char* buffer) const {
        memcpy(buffer, this, sizeof(Film));
    }

    void deserialize(const char* buffer) {
        memcpy(this, buffer, sizeof(Film));
    }

    static size_t getSerializedSize() {
        return sizeof(Film);
    }

    int getId() const {
        return film_id;
    }
};
