#pragma once

#include <vector>
#include <cstring>

using namespace std;

struct HashBucket {
    char username[32];
    vector<int> logIds;
    bool occupied;

    HashBucket() : occupied(false) {
        memset(username, 0, sizeof(username));
    }
};

class UserHashIndex {
private:
    static const int TABLE_SIZE = 1024;
    HashBucket table[TABLE_SIZE];

    unsigned long djb2Hash(const char* str) const {
        unsigned long hash = 5381;
        int c;
        while ((c = *str++)) {
            hash = ((hash << 5) + hash) + c;
        }
        return hash % TABLE_SIZE;
    }

    int findSlot(const char* username) const {
        unsigned long hash = djb2Hash(username);
        int index = hash;
        int probeCount = 0;

        while (probeCount < TABLE_SIZE) {
            if (!table[index].occupied) {
                return index;
            }
            if (strcmp(table[index].username, username) == 0) {
                return index;
            }
            index = (index + 1) % TABLE_SIZE;
            probeCount++;
        }
        return -1;
    }

public:
    UserHashIndex() {}

    void addEntry(const char* username, int logId) {
        int slot = findSlot(username);
        if (slot == -1) return;

        if (!table[slot].occupied) {
            strncpy(table[slot].username, username, sizeof(table[slot].username) - 1);
            table[slot].username[sizeof(table[slot].username) - 1] = '\0';
            table[slot].occupied = true;
        }
        table[slot].logIds.push_back(logId);
    }

    void removeEntry(const char* username, int logId) {
        int slot = findSlot(username);
        if (slot == -1 || !table[slot].occupied) return;
        
        if (strcmp(table[slot].username, username) == 0) {
            for (size_t i = 0; i < table[slot].logIds.size(); i++) {
                if (table[slot].logIds[i] == logId) {
                    table[slot].logIds.erase(table[slot].logIds.begin() + i);
                    break;
                }
            }
            if (table[slot].logIds.empty()) {
                table[slot].occupied = false;
                memset(table[slot].username, 0, sizeof(table[slot].username));
            }
        }
    }

    vector<int> getUserLogs(const char* username) const {
        int slot = findSlot(username);
        if (slot != -1 && table[slot].occupied && strcmp(table[slot].username, username) == 0) {
            return table[slot].logIds;
        }
        return vector<int>();
    }

    bool hasUser(const char* username) const {
        int slot = findSlot(username);
        return (slot != -1 && table[slot].occupied && strcmp(table[slot].username, username) == 0);
    }

    void clear() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i].occupied = false;
            table[i].logIds.clear();
        }
    }
};
