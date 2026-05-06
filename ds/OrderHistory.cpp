#include "OrderHistory.h"
#include <iostream>

using namespace std;

OrderHistory::OrderHistory() {
    topNode = nullptr;
    count = 0;
}

OrderHistory::~OrderHistory() {
    while (topNode != nullptr) {
        HistoryNode* temp = topNode;
        topNode = topNode->next;
        delete temp;
    }
    count = 0;
}

// push to stack - LIFO
void OrderHistory::push(Order o) {
    HistoryNode* newNode = new HistoryNode;
    newNode->data = o;
    newNode->next = topNode;
    topNode = newNode;
    count++;
    cout << "[Stack] Order #" << o.id << " pushed to history (stack size: " << count << ")\n";
}

// pop from stack for undo
Order OrderHistory::undo() {
    if (topNode == nullptr) {
        return Order{-1, "", "", false, PENDING, 0, -1};
    }

    HistoryNode* temp = topNode;
    Order order = temp->data;
    topNode = topNode->next;
    delete temp;
    count--;

    cout << "[Stack] Undo: Order #" << order.id << " restored (stack size: " << count << ")\n";

    return order;
}

bool OrderHistory::empty() {
    return topNode == nullptr;
}

int OrderHistory::size() {
    return count;
}