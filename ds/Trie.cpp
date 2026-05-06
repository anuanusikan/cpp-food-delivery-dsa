#include "Trie.h"
#include <cctype>

Trie::Trie() {
    root = new TrieNode();
}

Trie::~Trie() {
    cleanup(root);
}

void Trie::cleanup(TrieNode* node) {
    if (node == nullptr) {
        return;
    }

    for (int i = 0; i < 128; i++) {
        cleanup(node->children[i]);
    }

    delete node;
}

void Trie::insert(string word) {
    TrieNode* current = root;

    for (char c : word) {
        c = tolower(c);

        int index = (int)c;

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

bool Trie::search(string word) {
    TrieNode* current = root;

    for (char c : word) {
        c = tolower(c);

        int index = (int)c;

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

bool Trie::startsWith(string prefix) {
    TrieNode* current = root;

    for (char c : prefix) {
        c = tolower(c);

        int index = (int)c;

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