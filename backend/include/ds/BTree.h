#pragma once

#include "../models/FilmLog.h"
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

#define BTREE_ORDER 5

struct BTreeNode {
    bool isLeaf;
    int numKeys;
    FilmLog keys[BTREE_ORDER - 1];
    long children[BTREE_ORDER];
    long nodePos;

    BTreeNode() : isLeaf(true), numKeys(0), nodePos(-1) {
        for (int i = 0; i < BTREE_ORDER; i++) {
            children[i] = -1;
        }
    }

    void serialize(char* buffer) const {
        size_t offset = 0;
        memcpy(buffer + offset, &isLeaf, sizeof(bool));
        offset += sizeof(bool);
        memcpy(buffer + offset, &numKeys, sizeof(int));
        offset += sizeof(int);
        memcpy(buffer + offset, keys, sizeof(FilmLog) * (BTREE_ORDER - 1));
        offset += sizeof(FilmLog) * (BTREE_ORDER - 1);
        memcpy(buffer + offset, children, sizeof(long) * BTREE_ORDER);
        offset += sizeof(long) * BTREE_ORDER;
        memcpy(buffer + offset, &nodePos, sizeof(long));
    }

    void deserialize(const char* buffer) {
        size_t offset = 0;
        memcpy(&isLeaf, buffer + offset, sizeof(bool));
        offset += sizeof(bool);
        memcpy(&numKeys, buffer + offset, sizeof(int));
        offset += sizeof(int);
        memcpy(keys, buffer + offset, sizeof(FilmLog) * (BTREE_ORDER - 1));
        offset += sizeof(FilmLog) * (BTREE_ORDER - 1);
        memcpy(children, buffer + offset, sizeof(long) * BTREE_ORDER);
        offset += sizeof(long) * BTREE_ORDER;
        memcpy(&nodePos, buffer + offset, sizeof(long));
    }

    static size_t getSerializedSize() {
        return sizeof(bool) + sizeof(int) + sizeof(FilmLog) * (BTREE_ORDER - 1) 
               + sizeof(long) * BTREE_ORDER + sizeof(long);
    }
};

class BTree {
private:
    fstream file;
    long rootPos;
    long nextPos;
    string filename;

    long allocateNode() {
        long pos = nextPos;
        nextPos += BTreeNode::getSerializedSize();
        return pos;
    }

    void writeNode(const BTreeNode& node) {
        char buffer[BTreeNode::getSerializedSize()];
        node.serialize(buffer);
        file.seekp(node.nodePos);
        file.write(buffer, BTreeNode::getSerializedSize());
        file.flush();
    }

    BTreeNode readNode(long pos) {
        char buffer[BTreeNode::getSerializedSize()];
        file.seekg(pos);
        file.read(buffer, BTreeNode::getSerializedSize());
        BTreeNode node;
        node.deserialize(buffer);
        return node;
    }

    void splitChild(BTreeNode& parent, int index) {
        BTreeNode fullChild = readNode(parent.children[index]);
        BTreeNode newChild;
        newChild.isLeaf = fullChild.isLeaf;
        newChild.numKeys = BTREE_ORDER / 2 - 1;
        newChild.nodePos = allocateNode();

        for (int i = 0; i < BTREE_ORDER / 2 - 1; i++) {
            newChild.keys[i] = fullChild.keys[i + BTREE_ORDER / 2];
        }

        if (!fullChild.isLeaf) {
            for (int i = 0; i < BTREE_ORDER / 2; i++) {
                newChild.children[i] = fullChild.children[i + BTREE_ORDER / 2];
            }
        }

        FilmLog midKey = fullChild.keys[BTREE_ORDER / 2 - 1];
        fullChild.numKeys = BTREE_ORDER / 2 - 1;

        for (int i = parent.numKeys; i > index; i--) {
            parent.children[i + 1] = parent.children[i];
        }
        parent.children[index + 1] = newChild.nodePos;

        for (int i = parent.numKeys - 1; i >= index; i--) {
            parent.keys[i + 1] = parent.keys[i];
        }
        parent.keys[index] = midKey;
        parent.numKeys++;

        writeNode(fullChild);
        writeNode(newChild);
        writeNode(parent);
    }

    void insertNonFull(BTreeNode& node, const FilmLog& record) {
        int i = node.numKeys - 1;

        if (node.isLeaf) {
            while (i >= 0 && record.logId < node.keys[i].logId) {
                node.keys[i + 1] = node.keys[i];
                i--;
            }
            node.keys[i + 1] = record;
            node.numKeys++;
            writeNode(node);
        } else {
            while (i >= 0 && record.logId < node.keys[i].logId) {
                i--;
            }
            i++;
            BTreeNode child = readNode(node.children[i]);
            if (child.numKeys == BTREE_ORDER - 1) {
                splitChild(node, i);
                if (record.logId > node.keys[i].logId) {
                    i++;
                }
                child = readNode(node.children[i]);
            }
            insertNonFull(child, record);
        }
    }

    bool searchNode(const BTreeNode& node, int logId, FilmLog& result) {
        int i = 0;
        while (i < node.numKeys && logId > node.keys[i].logId) {
            i++;
        }

        if (i < node.numKeys && logId == node.keys[i].logId) {
            result = node.keys[i];
            return true;
        }

        if (node.isLeaf) {
            return false;
        }

        BTreeNode child = readNode(node.children[i]);
        return searchNode(child, logId, result);
    }

    void collectAllRecords(const BTreeNode& node, vector<FilmLog>& records) {
        for (int i = 0; i < node.numKeys; i++) {
            records.push_back(node.keys[i]);
        }

        if (!node.isLeaf) {
            for (int i = 0; i <= node.numKeys; i++) {
                if (node.children[i] != -1) {
                    BTreeNode child = readNode(node.children[i]);
                    collectAllRecords(child, records);
                }
            }
        }
    }

    void removeFromLeaf(BTreeNode& node, int idx) {
        for (int i = idx + 1; i < node.numKeys; i++) {
            node.keys[i - 1] = node.keys[i];
        }
        node.numKeys--;
        writeNode(node);
    }

    bool deleteKey(BTreeNode& node, int logId) {
        int idx = 0;
        while (idx < node.numKeys && node.keys[idx].logId < logId) {
            idx++;
        }

        if (idx < node.numKeys && node.keys[idx].logId == logId) {
            if (node.isLeaf) {
                removeFromLeaf(node, idx);
                return true;
            }
        }

        if (node.isLeaf) {
            return false;
        }

        BTreeNode child = readNode(node.children[idx]);
        bool found = deleteKey(child, logId);
        return found;
    }

public:
    BTree(const string& fname) : filename(fname), rootPos(0), nextPos(sizeof(long) * 2) {
        file.open(filename, ios::in | ios::out | ios::binary);
        
        if (!file.is_open()) {
            file.clear();
            file.open(filename, ios::out | ios::binary);
            file.close();
            file.open(filename, ios::in | ios::out | ios::binary);
            
            BTreeNode root;
            root.nodePos = nextPos;
            nextPos += BTreeNode::getSerializedSize();
            rootPos = root.nodePos;
            
            file.seekp(0);
            file.write(reinterpret_cast<char*>(&rootPos), sizeof(long));
            file.write(reinterpret_cast<char*>(&nextPos), sizeof(long));
            writeNode(root);
        } else {
            file.seekg(0);
            file.read(reinterpret_cast<char*>(&rootPos), sizeof(long));
            file.read(reinterpret_cast<char*>(&nextPos), sizeof(long));
        }
    }

    ~BTree() {
        if (file.is_open()) {
            file.seekp(0);
            file.write(reinterpret_cast<char*>(&rootPos), sizeof(long));
            file.write(reinterpret_cast<char*>(&nextPos), sizeof(long));
            file.close();
        }
    }

    void insert(const FilmLog& record) {
        BTreeNode root = readNode(rootPos);

        if (root.numKeys == BTREE_ORDER - 1) {
            BTreeNode newRoot;
            newRoot.isLeaf = false;
            newRoot.numKeys = 0;
            newRoot.nodePos = allocateNode();
            newRoot.children[0] = rootPos;

            splitChild(newRoot, 0);
            insertNonFull(newRoot, record);
            rootPos = newRoot.nodePos;
        } else {
            insertNonFull(root, record);
        }
    }

    bool search(int logId, FilmLog& result) {
        BTreeNode root = readNode(rootPos);
        return searchNode(root, logId, result);
    }

    vector<FilmLog> getAllRecords() {
        vector<FilmLog> records;
        BTreeNode root = readNode(rootPos);
        collectAllRecords(root, records);
        return records;
    }

    int getMaxLogId() {
        vector<FilmLog> records = getAllRecords();
        int maxId = 0;
        for (const auto& rec : records) {
            if (rec.logId > maxId) {
                maxId = rec.logId;
            }
        }
        return maxId;
    }

    bool deleteRecord(int logId) {
        BTreeNode root = readNode(rootPos);
        return deleteKey(root, logId);
    }

    bool updateRecord(int logId, const FilmLog& updatedLog) {
        BTreeNode root = readNode(rootPos);
        if (updateInTree(root, logId, updatedLog)) {
            return true;
        }
        return false;
    }

    bool updateInTree(BTreeNode& node, int logId, const FilmLog& updatedLog) {
        int idx = 0;
        while (idx < node.numKeys && node.keys[idx].logId < logId) {
            idx++;
        }

        if (idx < node.numKeys && node.keys[idx].logId == logId) {
            node.keys[idx] = updatedLog;
            writeNode(node);
            return true;
        }

        if (node.isLeaf) {
            return false;
        }

        BTreeNode child = readNode(node.children[idx]);
        return updateInTree(child, logId, updatedLog);
    }
};
