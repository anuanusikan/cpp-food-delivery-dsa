#include "OrderHistory.h"

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

void OrderHistory::push(Order o) {
    HistoryNode* newNode = new HistoryNode;
    newNode->data = o;
    newNode->next = topNode;

    topNode = newNode;
    count++;
}

Order OrderHistory::undo() {
    if (topNode == nullptr) {
        return Order{-1, "", "", "", "", 0, false, PENDING, 0, -1, 0};
    }

    HistoryNode* temp = topNode;
    Order order = temp->data;

    topNode = topNode->next;
    delete temp;

    count--;

    return order;
}

bool OrderHistory::empty() {
    return topNode == nullptr;
}

int OrderHistory::size() {
    return count;
}