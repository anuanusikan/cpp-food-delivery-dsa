#include "HashMap.h"
#include <iostream>

using namespace std;

HashMap::HashMap() {
    // initialize all buckets to null
    for (int i = 0; i < SIZE; i++) {
        table[i] = nullptr;
    }
}

HashMap::~HashMap() {
    for (int i = 0; i < SIZE; i++) {
        HashNode* cur = table[i];
        while (cur != nullptr) {
            HashNode* tmp = cur;
            cur = cur->next;
            delete tmp;
        }
        table[i] = nullptr;
    }
}

// polynomial rolling hash - using 31 as multiplier
// size 101 is a prime number which reduces collisions
int HashMap::hashFunction(const string& key) const {
    int h = 0;
    for (char c : key) {
        h = (h * 31 + c) % SIZE;
    }
    return h;
}

void HashMap::insert(const string& key, const string& value) {
    int index = hashFunction(key);
    HashNode* cur = table[index];

    // check if key already exists and update
    while (cur != nullptr) {
        if (cur->key == key) {
            cur->value = value;
            return;
        }
        cur = cur->next;
    }

    // key not found - add new node at front (chaining)
    HashNode* newNode = new HashNode(key, value);
    newNode->next = table[index];
    table[index] = newNode;
}

bool HashMap::containsKey(const string& key) const {
    int index = hashFunction(key);
    HashNode* cur = table[index];

    while (cur != nullptr) {
        if (cur->key == key) return true;
        cur = cur->next;
    }

    return false;
}

string HashMap::getValue(const string& key) const {
    int index = hashFunction(key);
    HashNode* cur = table[index];

    while (cur != nullptr) {
        if (cur->key == key) return cur->value;
        cur = cur->next;
    }

    return "";  // not found
}

void HashMap::removeKey(const string& key) {
    int index = hashFunction(key);
    HashNode* cur = table[index];
    HashNode* previous = nullptr;

    while (cur != nullptr) {
        if (cur->key == key) {
            if (previous == nullptr) {
                table[index] = cur->next;
            } else {
                previous->next = cur->next;
            }
            delete cur;
            return;
        }
        previous = cur;
        cur = cur->next;
    }
}