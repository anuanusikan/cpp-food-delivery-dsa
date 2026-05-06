#include "Cart.h"
#include <iostream>

using namespace std;

CartManager::CartManager() {
    sessions = nullptr;
}

CartManager::~CartManager() {
    SessionCartNode* cur = sessions;
    while (cur != nullptr) {
        SessionCartNode* tmp = cur;
        cur = cur->next;
        freeCart(tmp->head);
        delete tmp;
    }
    sessions = nullptr;
}

SessionCartNode* CartManager::findSession(string sessionId) {
    SessionCartNode* cur = sessions;
    while (cur != nullptr) {
        if (cur->sessionId == sessionId) {
            return cur;
        }
        cur = cur->next;
    }
    return nullptr;
}

SessionCartNode* CartManager::createSession(string sessionId) {
    SessionCartNode* s = new SessionCartNode;
    s->sessionId = sessionId;
    s->head = nullptr;
    s->tail = nullptr;
    s->next = sessions;
    sessions = s;
    return s;
}

// frees all cart nodes starting from head
void CartManager::freeCart(CartNode* head) {
    CartNode* cur = head;
    while (cur != nullptr) {
        CartNode* tmp = cur;
        cur = cur->next;
        delete tmp;
    }
}

void CartManager::addItem(string sessionId, string item, int price) {
    SessionCartNode* session = findSession(sessionId);
    if (session == nullptr) {
        session = createSession(sessionId);
    }

    CartNode* newNode = new CartNode;
    newNode->item = item;
    newNode->price = price;
    newNode->next = nullptr;
    newNode->prev = nullptr;

    if (session->head == nullptr) {
        // cart was empty
        session->head = newNode;
        session->tail = newNode;
    } else {
        // append to tail using doubly linked list
        session->tail->next = newNode;
        newNode->prev = session->tail;
        session->tail = newNode;
    }

    cout << "[Cart] Added '" << item << "' (Rs." << price << ") to session cart\n";
}

// removes FIRST occurrence of itemName - uses prev pointer to relink
bool CartManager::removeItem(string sessionId, string itemName) {
    SessionCartNode* session = findSession(sessionId);
    if (session == nullptr) return false;

    CartNode* cur = session->head;
    while (cur != nullptr) {
        if (cur->item == itemName) {
            // use prev pointer to fix the link before this node
            if (cur->prev != nullptr) {
                cur->prev->next = cur->next;
            } else {
                // removing head
                session->head = cur->next;
            }

            if (cur->next != nullptr) {
                cur->next->prev = cur->prev;
            } else {
                // removing tail
                session->tail = cur->prev;
            }

            delete cur;
            cout << "[Cart] Removed '" << itemName << "' from session cart\n";
            return true;
        }
        cur = cur->next;
    }

    return false;  // item not found
}

CartNode* CartManager::getCart(string sessionId) {
    SessionCartNode* session = findSession(sessionId);
    if (session == nullptr) return nullptr;
    return session->head;
}

vector<pair<string, int>> CartManager::getCartItems(string sessionId) {
    vector<pair<string, int>> result;
    CartNode* cur = getCart(sessionId);
    while (cur != nullptr) {
        result.push_back({cur->item, cur->price});
        cur = cur->next;
    }
    return result;
}

string CartManager::getItemsAsString(string sessionId) {
    string result = "";
    CartNode* cur = getCart(sessionId);
    while (cur != nullptr) {
        result += cur->item;
        if (cur->next != nullptr) {
            result += ", ";
        }
        cur = cur->next;
    }
    return result;
}

int CartManager::getTotalCartItems() {
    int count = 0;
    SessionCartNode* s = sessions;
    while (s != nullptr) {
        CartNode* cur = s->head;
        while (cur != nullptr) {
            count++;
            cur = cur->next;
        }
        s = s->next;
    }
    return count;
}

int CartManager::getActiveSessionCount() {
    int count = 0;
    SessionCartNode* s = sessions;
    while (s != nullptr) {
        if (s->head != nullptr) count++;
        s = s->next;
    }
    return count;
}

bool CartManager::isCartEmpty(string sessionId) {
    return getCart(sessionId) == nullptr;
}

void CartManager::clearCart(string sessionId) {
    SessionCartNode* cur = sessions;
    SessionCartNode* prev = nullptr;

    while (cur != nullptr) {
        if (cur->sessionId == sessionId) {
            freeCart(cur->head);
            if (prev == nullptr) {
                sessions = cur->next;
            } else {
                prev->next = cur->next;
            }
            delete cur;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}