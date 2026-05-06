#pragma once
#include <string>
#include <vector>

using namespace std;

struct CartNode {
    string item;
    int price;
    CartNode* next;
    CartNode* prev;
};

struct SessionCartNode {
    string sessionId;
    CartNode* head;
    CartNode* tail;
    SessionCartNode* next;
};

class CartManager {
private:
    SessionCartNode* sessions;

    SessionCartNode* findSession(string sessionId);
    SessionCartNode* createSession(string sessionId);
    void deleteCartNodes(CartNode* head);

public:
    CartManager();
    ~CartManager();

    void addItem(string sessionId, string item, int price);
    CartNode* getCart(string sessionId);
    vector<pair<string, int>> getCartItems(string sessionId);
    string getItemsAsString(string sessionId);
    int getTotalCartItems();
    int getActiveSessionCount();
    bool isCartEmpty(string sessionId);
    void clearCart(string sessionId);
};