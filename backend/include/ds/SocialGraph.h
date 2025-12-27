#pragma once

#include <unordered_map>
#include <vector>
#include <fstream>
#include <cstring>

using namespace std;

// Social Graph using Adjacency List for directed follows
class SocialGraph {
private:
    unordered_map<int, vector<int>> adjacencyList; // userId -> list of followed userIds
    string filename;

    struct GraphHeader {
        int totalUsers;
        int totalConnections;
    };

public:
    SocialGraph(const string& file) : filename(file) {
        loadFromDisk();
    }

    ~SocialGraph() {
        saveToDisk();
    }

    // Follow a user (directed edge: follower -> target)
    bool followUser(int followerId, int targetId) {
        if (followerId == targetId) return false; // Can't follow yourself
        
        // Check if already following
        auto& following = adjacencyList[followerId];
        for (int id : following) {
            if (id == targetId) return false; // Already following
        }
        
        following.push_back(targetId);
        saveToDisk();
        return true;
    }

    // Unfollow a user
    bool unfollowUser(int followerId, int targetId) {
        auto it = adjacencyList.find(followerId);
        if (it == adjacencyList.end()) return false;
        
        auto& following = it->second;
        for (size_t i = 0; i < following.size(); i++) {
            if (following[i] == targetId) {
                following.erase(following.begin() + i);
                saveToDisk();
                return true;
            }
        }
        return false;
    }

    // Get list of users that this user follows
    vector<int> getFollowing(int userId) {
        auto it = adjacencyList.find(userId);
        if (it != adjacencyList.end()) {
            return it->second;
        }
        return vector<int>();
    }

    // Get list of users that follow this user
    vector<int> getFollowers(int userId) {
        vector<int> followers;
        for (const auto& pair : adjacencyList) {
            for (int followedId : pair.second) {
                if (followedId == userId) {
                    followers.push_back(pair.first);
                    break;
                }
            }
        }
        return followers;
    }

    // Check if follower follows target
    bool isFollowing(int followerId, int targetId) {
        auto it = adjacencyList.find(followerId);
        if (it == adjacencyList.end()) return false;
        
        for (int id : it->second) {
            if (id == targetId) return true;
        }
        return false;
    }

    // Get counts
    int getFollowingCount(int userId) {
        auto it = adjacencyList.find(userId);
        return (it != adjacencyList.end()) ? it->second.size() : 0;
    }

    int getFollowersCount(int userId) {
        return getFollowers(userId).size();
    }

    // Persistence
    void saveToDisk() {
        ofstream file(filename, ios::binary);
        if (!file.is_open()) return;

        // Count total connections
        int totalConnections = 0;
        for (const auto& pair : adjacencyList) {
            totalConnections += pair.second.size();
        }

        // Write header
        GraphHeader header;
        header.totalUsers = adjacencyList.size();
        header.totalConnections = totalConnections;
        file.write(reinterpret_cast<char*>(&header), sizeof(GraphHeader));

        // Write each user's following list
        for (const auto& pair : adjacencyList) {
            int userId = pair.first;
            int followingCount = pair.second.size();
            
            file.write(reinterpret_cast<const char*>(&userId), sizeof(int));
            file.write(reinterpret_cast<const char*>(&followingCount), sizeof(int));
            
            for (int followedId : pair.second) {
                file.write(reinterpret_cast<const char*>(&followedId), sizeof(int));
            }
        }

        file.close();
    }

    void loadFromDisk() {
        ifstream file(filename, ios::binary);
        if (!file.is_open()) return; // File doesn't exist yet

        GraphHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(GraphHeader));

        adjacencyList.clear();

        for (int i = 0; i < header.totalUsers; i++) {
            int userId, followingCount;
            file.read(reinterpret_cast<char*>(&userId), sizeof(int));
            file.read(reinterpret_cast<char*>(&followingCount), sizeof(int));

            vector<int> following;
            for (int j = 0; j < followingCount; j++) {
                int followedId;
                file.read(reinterpret_cast<char*>(&followedId), sizeof(int));
                following.push_back(followedId);
            }

            adjacencyList[userId] = following;
        }

        file.close();
    }
};
