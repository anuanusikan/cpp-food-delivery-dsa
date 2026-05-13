#pragma once
#include <string>

using namespace std;

enum OrderStatus {
    PENDING,
    PREPARING,
    READY,
    ASSIGNED,
    IN_DELIVERY,
    DELIVERED,
    CANCELLED
};

struct Order {
    int id;
    string sessionId;
    string items;
    string address;
    string nearestLocation;
    int userDistance;
    bool isVIP;
    OrderStatus status;
    long long timestamp;
    int assignedDriverId;
    int distanceFromRestaurant;
};