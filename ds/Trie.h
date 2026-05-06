#pragma once
#include <string>

using namespace std;

struct TrieNode {
    TrieNode* children[128];
    bool isEnd;

    TrieNode() {
        isEnd = false;

        for (int i = 0; i < 128; i++) {
            children[i] = nullptr;
        }
    }
};

class Trie {
private:
    TrieNode* root;

    void cleanup(TrieNode* node);

public:
    Trie();
    ~Trie();

    void insert(string word);
    bool search(string word);
    bool startsWith(string prefix);
};