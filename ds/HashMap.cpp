#include "HashMap.h"

HashMap::HashMap() {
    for (int i = 0; i < SIZE; i++) {
        table[i] = nullptr;
    }
}

HashMap::~HashMap() {
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

int HashMap::hashFunction(const string& key) const {
    int hashValue = 0;

    for (char c : key) {
        hashValue = (hashValue * 31 + c) % SIZE;
    }

    return hashValue;
}

void HashMap::insert(const string& key, const string& value) {
    int index = hashFunction(key);

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

bool HashMap::containsKey(const string& key) const {
    int index = hashFunction(key);

    HashNode* current = table[index];

    while (current != nullptr) {
        if (current->key == key) {
            return true;
        }

        current = current->next;
    }

    return false;
}

string HashMap::getValue(const string& key) const {
    int index = hashFunction(key);

    HashNode* current = table[index];

    while (current != nullptr) {
        if (current->key == key) {
            return current->value;
        }

        current = current->next;
    }

    return "";
}

void HashMap::removeKey(const string& key) {
    int index = hashFunction(key);

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