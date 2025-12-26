#pragma once

#include <cstring>
#include <ctime>

using namespace std;

struct User {
    int user_id;
    char username[32];
    char email[64];
    char password_hash[64];
    char bio[256];
    long join_date;
    bool isAdmin;
    int avatar_id;             // New: Profile avatar selection (1-10)

    User() : user_id(0), join_date(0), isAdmin(false), avatar_id(1) {
        memset(username, 0, sizeof(username));
        memset(email, 0, sizeof(email));
        memset(password_hash, 0, sizeof(password_hash));
        memset(bio, 0, sizeof(bio));
    }

    User(int id, const char* uname, const char* em, const char* pass, const char* b, bool admin = false, int avatar = 1) 
        : user_id(id), join_date(time(nullptr)), isAdmin(admin), avatar_id(avatar) {
        strncpy(username, uname, sizeof(username) - 1);
        username[sizeof(username) - 1] = '\0';
        strncpy(email, em, sizeof(email) - 1);
        email[sizeof(email) - 1] = '\0';
        strncpy(password_hash, pass, sizeof(password_hash) - 1);
        password_hash[sizeof(password_hash) - 1] = '\0';
        strncpy(bio, b, sizeof(bio) - 1);
        bio[sizeof(bio) - 1] = '\0';
    }

    void serialize(char* buffer) const {
        memcpy(buffer, this, sizeof(User));
    }

    void deserialize(const char* buffer) {
        memcpy(this, buffer, sizeof(User));
    }

    static size_t getSerializedSize() {
        return sizeof(User);
    }

    int getId() const {
        return user_id;
    }
};
