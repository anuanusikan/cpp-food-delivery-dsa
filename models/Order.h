#pragma once
#include <string>

using namespace std;

enum OrderStatus {
    PENDING,
    PREPARING,
    READY,
    ASSIGNED,
    IN_DELIVERY,
    DELIVERED
};

struct Order {
    int id;
    string items;
    string address;
    bool isVIP;
    OrderStatus status;
    long long timestamp;
    int assignedDriverId;

    bool operator<(const Order& other) const {
        if (isVIP != other.isVIP) {
            return !isVIP; // VIP orders get higher priority
        }

        return id > other.id; // smaller ID first for same priority
    }
};