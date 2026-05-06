#include "Cart.h"

CartManager::CartManager() {
    sessions = nullptr;
}

CartManager::~CartManager() {
    SessionCartNode* currentSession = sessions;

    while (currentSession != nullptr) {
        SessionCartNode* tempSession = currentSession;
        currentSession = currentSession->next;

        deleteCartNodes(tempSession->head);
        delete tempSession;
    }

    sessions = nullptr;
}

SessionCartNode* CartManager::findSession(string sessionId) {
    SessionCartNode* current = sessions;

    while (current != nullptr) {
        if (current->sessionId == sessionId) {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}

SessionCartNode* CartManager::createSession(string sessionId) {
    SessionCartNode* newSession = new SessionCartNode;
    newSession->sessionId = sessionId;
    newSession->head = nullptr;
    newSession->tail = nullptr;
    newSession->next = sessions;

    sessions = newSession;

    return newSession;
}

void CartManager::deleteCartNodes(CartNode* head) {
    CartNode* current = head;

    while (current != nullptr) {
        CartNode* temp = current;
        current = current->next;
        delete temp;
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
        session->head = newNode;
        session->tail = newNode;
    } else {
        session->tail->next = newNode;
        newNode->prev = session->tail;
        session->tail = newNode;
    }
}

CartNode* CartManager::getCart(string sessionId) {
    SessionCartNode* session = findSession(sessionId);

    if (session == nullptr) {
        return nullptr;
    }

    return session->head;
}

vector<pair<string, int>> CartManager::getCartItems(string sessionId) {
    vector<pair<string, int>> result;

    CartNode* current = getCart(sessionId);

    while (current != nullptr) {
        result.push_back({current->item, current->price});
        current = current->next;
    }

    return result;
}

string CartManager::getItemsAsString(string sessionId) {
    string result = "";

    CartNode* current = getCart(sessionId);

    while (current != nullptr) {
        result += current->item;

        if (current->next != nullptr) {
            result += ", ";
        }

        current = current->next;
    }

    return result;
}

int CartManager::getTotalCartItems() {
    int count = 0;

    SessionCartNode* session = sessions;

    while (session != nullptr) {
        CartNode* current = session->head;

        while (current != nullptr) {
            count++;
            current = current->next;
        }

        session = session->next;
    }

    return count;
}

int CartManager::getActiveSessionCount() {
    int count = 0;

    SessionCartNode* session = sessions;

    while (session != nullptr) {
        if (session->head != nullptr) {
            count++;
        }

        session = session->next;
    }

    return count;
}

bool CartManager::isCartEmpty(string sessionId) {
    return getCart(sessionId) == nullptr;
}

void CartManager::clearCart(string sessionId) {
    SessionCartNode* current = sessions;
    SessionCartNode* previous = nullptr;

    while (current != nullptr) {
        if (current->sessionId == sessionId) {
            deleteCartNodes(current->head);

            if (previous == nullptr) {
                sessions = current->next;
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