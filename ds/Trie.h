#pragma once
#include <string>
#include <cctype>

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

    void deleteNodes(TrieNode* node) {
        if (node == nullptr) {
            return;
        }

        for (int i = 0; i < 128; i++) {
            deleteNodes(node->children[i]);
        }

        delete node;
    }

public:
    Trie() {
        root = new TrieNode();
    }

    ~Trie() {
        deleteNodes(root);
    }

    void insert(string word) {
        TrieNode* current = root;

        for (char ch : word) {
            ch = tolower(ch);

            int index = (int)ch;

            if (index < 0 || index >= 128) {
                continue;
            }

            if (current->children[index] == nullptr) {
                current->children[index] = new TrieNode();
            }

            current = current->children[index];
        }

        current->isEnd = true;
    }

    bool search(string word) {
        TrieNode* current = root;

        for (char ch : word) {
            ch = tolower(ch);

            int index = (int)ch;

            if (index < 0 || index >= 128) {
                return false;
            }

            if (current->children[index] == nullptr) {
                return false;
            }

            current = current->children[index];
        }

        return current->isEnd;
    }

    bool startsWith(string prefix) {
        TrieNode* current = root;

        for (char ch : prefix) {
            ch = tolower(ch);

            int index = (int)ch;

            if (index < 0 || index >= 128) {
                return false;
            }

            if (current->children[index] == nullptr) {
                return false;
            }

            current = current->children[index];
        }

        return true;
    }
};