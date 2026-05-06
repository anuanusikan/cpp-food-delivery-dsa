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
    OrderHistory();
    ~OrderHistory();

    void push(Order o);
    Order undo();
    bool empty();
    int size();
};