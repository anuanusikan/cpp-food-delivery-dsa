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

    // Final dynamic customer location, example: Customer_1000
    string address;

    // Nearest fixed graph node selected by user, example: Town Hall
    string nearestLocation;

    // Distance from nearest fixed location to customer
    int userDistance;

    bool isVIP;
    OrderStatus status;
    long long timestamp;
    int assignedDriverId;
    int distanceFromRestaurant;
};