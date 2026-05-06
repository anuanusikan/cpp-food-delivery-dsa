#pragma once
#include <string>
#include <vector>

using namespace std;

// each item in the cart is stored as a node
// using doubly linked list so we can remove from anywhere easily
struct CartNode {
    string item;
    int price;
    CartNode* next;
    CartNode* prev;  // needed for removal without full traversal
};

// each user session gets its own linked list of cart nodes
struct SessionCartNode {
    string sessionId;
    CartNode* head;
    CartNode* tail;
    SessionCartNode* next;
};

class CartManager {
private:
    SessionCartNode* sessions;  // linked list of all sessions

    SessionCartNode* findSession(string sessionId);
    SessionCartNode* createSession(string sessionId);
    void freeCart(CartNode* head);  // helper to delete all cart nodes

public:
    CartManager();
    ~CartManager();

    void addItem(string sessionId, string item, int price);
    bool removeItem(string sessionId, string itemName);  // uses prev pointer
    CartNode* getCart(string sessionId);
    vector<pair<string, int>> getCartItems(string sessionId);
    string getItemsAsString(string sessionId);
    int getTotalCartItems();
    int getActiveSessionCount();
    bool isCartEmpty(string sessionId);
    void clearCart(string sessionId);
};