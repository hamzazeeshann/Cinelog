#pragma once

#include <cstring>
#include <vector>

using namespace std;

template<typename K, typename V>
struct HashNode {
    K key;
    V value;
    HashNode* next;

    HashNode(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
};

template<typename K, typename V>
class HashMap {
private:
    HashNode<K, V>** buckets;
    int capacity;
    int size;
    static constexpr float LOAD_FACTOR = 0.7f;

    unsigned long hashFunction(const char* str) const {
        unsigned long hash = 5381;
        int c;
        while ((c = *str++)) {
            hash = ((hash << 5) + hash) + c;
        }
        return hash % capacity;
    }

    void resize() {
        int oldCapacity = capacity;
        HashNode<K, V>** oldBuckets = buckets;

        capacity *= 2;
        buckets = new HashNode<K, V>*[capacity];
        for (int i = 0; i < capacity; i++) {
            buckets[i] = nullptr;
        }
        size = 0;

        for (int i = 0; i < oldCapacity; i++) {
            HashNode<K, V>* node = oldBuckets[i];
            while (node != nullptr) {
                HashNode<K, V>* next = node->next;
                insertRehash(node->key, node->value);
                delete node;
                node = next;
            }
        }

        delete[] oldBuckets;
    }

    void insertRehash(const K& key, const V& value) {
        unsigned long index = hashFunction(key);
        HashNode<K, V>* newNode = new HashNode<K, V>(key, value);
        newNode->next = buckets[index];
        buckets[index] = newNode;
        size++;
    }

public:
    HashMap(int initialCapacity = 16) : capacity(initialCapacity), size(0) {
        buckets = new HashNode<K, V>*[capacity];
        for (int i = 0; i < capacity; i++) {
            buckets[i] = nullptr;
        }
    }

    ~HashMap() {
        clear();
        delete[] buckets;
    }

    void insert(const K& key, const V& value) {
        if (static_cast<float>(size + 1) / capacity > LOAD_FACTOR) {
            resize();
        }

        unsigned long index = hashFunction(key);
        HashNode<K, V>* node = buckets[index];

        while (node != nullptr) {
            if (strcmp(node->key, key) == 0) {
                node->value = value;
                return;
            }
            node = node->next;
        }

        HashNode<K, V>* newNode = new HashNode<K, V>(key, value);
        newNode->next = buckets[index];
        buckets[index] = newNode;
        size++;
    }

    bool find(const K& key, V& result) const {
        unsigned long index = hashFunction(key);
        HashNode<K, V>* node = buckets[index];

        while (node != nullptr) {
            if (strcmp(node->key, key) == 0) {
                result = node->value;
                return true;
            }
            node = node->next;
        }
        return false;
    }

    V* get(const K& key) {
        unsigned long index = hashFunction(key);
        HashNode<K, V>* node = buckets[index];

        while (node != nullptr) {
            if (strcmp(node->key, key) == 0) {
                return &(node->value);
            }
            node = node->next;
        }
        return nullptr;
    }

    bool remove(const K& key) {
        unsigned long index = hashFunction(key);
        HashNode<K, V>* node = buckets[index];
        HashNode<K, V>* prev = nullptr;

        while (node != nullptr) {
            if (strcmp(node->key, key) == 0) {
                if (prev == nullptr) {
                    buckets[index] = node->next;
                } else {
                    prev->next = node->next;
                }
                delete node;
                size--;
                return true;
            }
            prev = node;
            node = node->next;
        }
        return false;
    }

    void clear() {
        for (int i = 0; i < capacity; i++) {
            HashNode<K, V>* node = buckets[i];
            while (node != nullptr) {
                HashNode<K, V>* temp = node;
                node = node->next;
                delete temp;
            }
            buckets[i] = nullptr;
        }
        size = 0;
    }

    int getSize() const {
        return size;
    }

    int getCapacity() const {
        return capacity;
    }
};
