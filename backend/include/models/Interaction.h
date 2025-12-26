#pragma once

#include <cstring>

using namespace std;

struct Interaction {
    int interaction_id;
    int user_id;
    int film_id;
    int type; // 1=Like/Favorite, 2=Watchlist

    Interaction() : interaction_id(0), user_id(0), film_id(0), type(0) {}

    Interaction(int iid, int uid, int fid, int t) 
        : interaction_id(iid), user_id(uid), film_id(fid), type(t) {}

    int getId() const { return interaction_id; }

    void serialize(char* buffer) const {
        memcpy(buffer, this, sizeof(Interaction));
    }

    void deserialize(const char* buffer) {
        memcpy(this, buffer, sizeof(Interaction));
    }
};
