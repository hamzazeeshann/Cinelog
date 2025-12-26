#pragma once

#include <cstring>

using namespace std;

struct Film {
    int film_id;
    int tmdb_id;
    char title[64];
    int release_year;
    int runtime;
    char director[64];
    int genre_ids[3];
    char poster_path[64];      // New: TMDB poster path
    char backdrop_path[64];    // New: TMDB backdrop path
    char tagline[128];         // New: Movie tagline
    float vote_average;        // New: TMDB rating
    char cast_summary[256];    // Kept for compatibility

    Film() : film_id(0), tmdb_id(0), release_year(0), runtime(0), vote_average(0.0f) {
        memset(title, 0, sizeof(title));
        memset(director, 0, sizeof(director));
        memset(genre_ids, 0, sizeof(genre_ids));
        memset(poster_path, 0, sizeof(poster_path));
        memset(backdrop_path, 0, sizeof(backdrop_path));
        memset(tagline, 0, sizeof(tagline));
        memset(cast_summary, 0, sizeof(cast_summary));
    }

    Film(int fid, int tid, const char* t, int year, int rt, const char* dir, 
         const char* poster, const char* backdrop, const char* tag, float rating) 
        : film_id(fid), tmdb_id(tid), release_year(year), runtime(rt), vote_average(rating) {
        strncpy(title, t, sizeof(title) - 1);
        title[sizeof(title) - 1] = '\0';
        strncpy(director, dir, sizeof(director) - 1);
        director[sizeof(director) - 1] = '\0';
        strncpy(poster_path, poster, sizeof(poster_path) - 1);
        poster_path[sizeof(poster_path) - 1] = '\0';
        strncpy(backdrop_path, backdrop, sizeof(backdrop_path) - 1);
        backdrop_path[sizeof(backdrop_path) - 1] = '\0';
        strncpy(tagline, tag, sizeof(tagline) - 1);
        tagline[sizeof(tagline) - 1] = '\0';
        memset(genre_ids, 0, sizeof(genre_ids));
        memset(cast_summary, 0, sizeof(cast_summary));
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
