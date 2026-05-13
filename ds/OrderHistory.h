#pragma once
#include "../models/Order.h"

struct HistoryNode {
    Order data;
    HistoryNode* next;
};

class OrderHistory {
private:
    HistoryNode* topNode;
    int count;

public:
    OrderHistory() {
        topNode = nullptr;
        count = 0;
    }

    ~OrderHistory() {
        while (topNode != nullptr) {
            HistoryNode* temp = topNode;
            topNode = topNode->next;
            delete temp;
        }

        count = 0;
    }

    void push(Order order) {
        HistoryNode* newNode = new HistoryNode;
        newNode->data = order;
        newNode->next = topNode;

        topNode = newNode;
        count++;
    }

    Order undo() {
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

    bool empty() {
        return topNode == nullptr;
    }

    int size() {
        return count;
    }
};