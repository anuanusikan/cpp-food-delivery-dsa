#pragma once
#include <vector>
#include "../models/Order.h"

using namespace std;

class OrderQueue {
private:
    vector<Order> heap;

    bool higherPriority(const Order& a, const Order& b);
    void heapifyUp(int index);
    void heapifyDown(int index);
    void rebuildHeap();

public:
    void push(Order o);
    Order pop();

    bool removeById(int orderId);

    bool empty();
    int size();

    vector<Order> getAll();
};