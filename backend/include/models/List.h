#pragma once

#include <cstring>

using namespace std;

struct List {
    int list_id;
    int user_id;
    char title[64];
    char description[256];

    List() : list_id(0), user_id(0) {
        memset(title, 0, sizeof(title));
        memset(description, 0, sizeof(description));
    }

    List(int lid, int uid, const char* t, const char* desc) 
        : list_id(lid), user_id(uid) {
        strncpy(title, t, sizeof(title) - 1);
        title[sizeof(title) - 1] = '\0';
        strncpy(description, desc, sizeof(description) - 1);
        description[sizeof(description) - 1] = '\0';
    }

    void serialize(char* buffer) const {
        memcpy(buffer, this, sizeof(List));
    }

    void deserialize(const char* buffer) {
        memcpy(this, buffer, sizeof(List));
    }

    static size_t getSerializedSize() {
        return sizeof(List);
    }

    int getId() const {
        return list_id;
    }
};

struct ListEntry {
    int list_id;
    int film_id;
    int rank;

    ListEntry() : list_id(0), film_id(0), rank(0) {}

    ListEntry(int lid, int fid, int r) 
        : list_id(lid), film_id(fid), rank(r) {}

    void serialize(char* buffer) const {
        memcpy(buffer, this, sizeof(ListEntry));
    }

    void deserialize(const char* buffer) {
        memcpy(this, buffer, sizeof(ListEntry));
    }

    static size_t getSerializedSize() {
        return sizeof(ListEntry);
    }

    int getId() const {
        return list_id * 10000 + rank; // Composite key
    }
};
