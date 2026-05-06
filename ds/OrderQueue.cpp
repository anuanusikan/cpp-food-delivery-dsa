#include "OrderQueue.h"

bool OrderQueue::higherPriority(const Order& a, const Order& b) {
    if (a.isVIP != b.isVIP) {
        return a.isVIP;
    }

    return a.id < b.id;
}

void OrderQueue::heapifyUp(int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;

        if (higherPriority(heap[index], heap[parent])) {
            Order temp = heap[index];
            heap[index] = heap[parent];
            heap[parent] = temp;

            index = parent;
        } else {
            break;
        }
    }
}

void OrderQueue::heapifyDown(int index) {
    int n = heap.size();

    while (true) {
        int left = (index * 2) + 1;
        int right = (index * 2) + 2;
        int best = index;

        if (left < n && higherPriority(heap[left], heap[best])) {
            best = left;
        }

        if (right < n && higherPriority(heap[right], heap[best])) {
            best = right;
        }

        if (best != index) {
            Order temp = heap[index];
            heap[index] = heap[best];
            heap[best] = temp;

            index = best;
        } else {
            break;
        }
    }
}

void OrderQueue::push(Order o) {
    heap.push_back(o);
    heapifyUp(heap.size() - 1);
}

Order OrderQueue::pop() {
    if (heap.empty()) {
        return Order{-1, "", "", false, PENDING, 0, -1};
    }

    Order root = heap[0];

    heap[0] = heap[heap.size() - 1];
    heap.pop_back();

    if (!heap.empty()) {
        heapifyDown(0);
    }

    return root;
}

bool OrderQueue::empty() {
    return heap.empty();
}

int OrderQueue::size() {
    return heap.size();
}

vector<Order> OrderQueue::getAll() {
    vector<Order> copy = heap;
    vector<Order> result;

    OrderQueue tempQueue;

    for (Order o : copy) {
        tempQueue.push(o);
    }

    while (!tempQueue.empty()) {
        result.push_back(tempQueue.pop());
    }

    return result;
}