#pragma once
#include <string>

using namespace std;

struct HashNode {
    string key;
    string value;
    HashNode* next;

    HashNode(string k, string v) {
        key = k;
        value = v;
        next = nullptr;
    }
};

class HashMap {
private:
    static const int SIZE = 101;
    HashNode* table[SIZE];

    int getIndex(const string& key) const {
        int hashValue = 0;

        for (char ch : key) {
            hashValue = (hashValue * 31 + ch) % SIZE;
        }

        return hashValue;
    }

public:
    HashMap() {
        for (int i = 0; i < SIZE; i++) {
            table[i] = nullptr;
        }
    }

    ~HashMap() {
        for (int i = 0; i < SIZE; i++) {
            HashNode* current = table[i];

            while (current != nullptr) {
                HashNode* temp = current;
                current = current->next;
                delete temp;
            }

            table[i] = nullptr;
        }
    }

    void insert(const string& key, const string& value) {
        int index = getIndex(key);

        HashNode* current = table[index];

        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }

            current = current->next;
        }

        HashNode* newNode = new HashNode(key, value);
        newNode->next = table[index];
        table[index] = newNode;
    }

    bool containsKey(const string& key) const {
        int index = getIndex(key);

        HashNode* current = table[index];

        while (current != nullptr) {
            if (current->key == key) {
                return true;
            }

            current = current->next;
        }

        return false;
    }

    string getValue(const string& key) const {
        int index = getIndex(key);

        HashNode* current = table[index];

        while (current != nullptr) {
            if (current->key == key) {
                return current->value;
            }

            current = current->next;
        }

        return "";
    }

    void removeKey(const string& key) {
        int index = getIndex(key);

        HashNode* current = table[index];
        HashNode* previous = nullptr;

        while (current != nullptr) {
            if (current->key == key) {
                if (previous == nullptr) {
                    table[index] = current->next;
                } else {
                    previous->next = current->next;
                }

                delete current;
                return;
            }

            previous = current;
            current = current->next;
        }
    }
};