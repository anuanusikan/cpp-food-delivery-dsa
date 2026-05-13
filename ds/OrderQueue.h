#pragma once
#include <vector>
#include <string>
#include "../models/Order.h"

using namespace std;

class OrderQueue {
private:
    vector<Order> heap;

    bool hasHigherPriority(const Order& firstOrder, const Order& secondOrder) {
        if (firstOrder.isVIP != secondOrder.isVIP) {
            return firstOrder.isVIP;
        }

        if (firstOrder.timestamp != secondOrder.timestamp) {
            return firstOrder.timestamp < secondOrder.timestamp;
        }

        return firstOrder.distanceFromRestaurant < secondOrder.distanceFromRestaurant;
    }

    void moveUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;

            if (hasHigherPriority(heap[index], heap[parent])) {
                Order temp = heap[index];
                heap[index] = heap[parent];
                heap[parent] = temp;

                index = parent;
            } else {
                break;
            }
        }
    }

    void moveDown(int index) {
        int size = heap.size();

        while (true) {
            int left = (index * 2) + 1;
            int right = (index * 2) + 2;
            int best = index;

            if (left < size && hasHigherPriority(heap[left], heap[best])) {
                best = left;
            }

            if (right < size && hasHigherPriority(heap[right], heap[best])) {
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

    void rebuildHeap() {
        for (int i = (int)heap.size() / 2 - 1; i >= 0; i--) {
            moveDown(i);
        }
    }

public:
    void push(Order order) {
        heap.push_back(order);
        moveUp(heap.size() - 1);
    }

    Order pop() {
        if (heap.empty()) {
            return Order{-1, "", "", "", "", 0, false, PENDING, 0, -1, 0};
        }

        Order topOrder = heap[0];

        heap[0] = heap[heap.size() - 1];
        heap.pop_back();

        if (!heap.empty()) {
            moveDown(0);
        }

        return topOrder;
    }

    bool removeById(int orderId) {
        for (int i = 0; i < (int)heap.size(); i++) {
            if (heap[i].id == orderId) {
                heap[i] = heap[heap.size() - 1];
                heap.pop_back();

                if (!heap.empty()) {
                    rebuildHeap();
                }

                return true;
            }
        }

        return false;
    }

    bool removeByIdAndSession(int orderId, string sessionId) {
        for (int i = 0; i < (int)heap.size(); i++) {
            if (heap[i].id == orderId && heap[i].sessionId == sessionId) {
                heap[i] = heap[heap.size() - 1];
                heap.pop_back();

                if (!heap.empty()) {
                    rebuildHeap();
                }

                return true;
            }
        }

        return false;
    }

    vector<Order> getBySession(string sessionId) {
        vector<Order> result;

        for (Order order : heap) {
            if (order.sessionId == sessionId) {
                result.push_back(order);
            }
        }

        return result;
    }

    bool empty() {
        return heap.empty();
    }

    int size() {
        return heap.size();
    }

    vector<Order> getAll() {
        vector<Order> copy = heap;
        vector<Order> result;

        OrderQueue tempQueue;

        for (Order order : copy) {
            tempQueue.push(order);
        }

        while (!tempQueue.empty()) {
            result.push_back(tempQueue.pop());
        }

        return result;
    }
};