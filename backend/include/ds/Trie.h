#pragma once

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>

using namespace std;

struct TrieNode {
    map<char, TrieNode*> children;
    vector<int> filmIds;  // Store film IDs at this node
    bool isEndOfWord;
    
    TrieNode() : isEndOfWord(false) {}
    
    ~TrieNode() {
        for (auto& pair : children) {
            delete pair.second;
        }
    }
};

class Trie {
private:
    TrieNode* root;
    
    string toLowerCase(const string& str) {
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    void collectResults(TrieNode* node, vector<int>& results) {
        if (node->isEndOfWord) {
            for (int id : node->filmIds) {
                results.push_back(id);
            }
        }
        
        for (auto& pair : node->children) {
            collectResults(pair.second, results);
        }
    }

public:
    Trie() {
        root = new TrieNode();
    }
    
    ~Trie() {
        delete root;
    }
    
    void insert(const string& title, int filmId) {
        string lowerTitle = toLowerCase(title);
        TrieNode* current = root;
        
        for (char c : lowerTitle) {
            if (c == ' ') continue;  // Skip spaces for better search
            
            if (current->children.find(c) == current->children.end()) {
                current->children[c] = new TrieNode();
            }
            current = current->children[c];
        }
        
        current->isEndOfWord = true;
        current->filmIds.push_back(filmId);
    }
    
    vector<int> search(const string& prefix) {
        string lowerPrefix = toLowerCase(prefix);
        TrieNode* current = root;
        vector<int> results;
        
        // Navigate to the prefix node
        for (char c : lowerPrefix) {
            if (c == ' ') continue;
            
            if (current->children.find(c) == current->children.end()) {
                return results;  // Prefix not found
            }
            current = current->children[c];
        }
        
        // Collect all film IDs from this node down
        collectResults(current, results);
        
        return results;
    }
    
    bool isEmpty() const {
        return root->children.empty();
    }
};
