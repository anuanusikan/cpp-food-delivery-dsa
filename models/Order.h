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
};