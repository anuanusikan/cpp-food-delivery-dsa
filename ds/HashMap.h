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

    int hashFunction(const string& key) const;

public:
    HashMap();
    ~HashMap();

    void insert(const string& key, const string& value);
    bool containsKey(const string& key) const;
    string getValue(const string& key) const;
    void removeKey(const string& key);
};