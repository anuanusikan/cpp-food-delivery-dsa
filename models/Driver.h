#pragma once
#include <string>
#include <vector>

using namespace std;

enum DriverStatus {
    AVAILABLE,
    ON_DELIVERY
};

struct Driver {
    int driverId;
    string name;
    DriverStatus status;

    string currentLocation;
    int activeOrderCount;
    int completedDeliveries;

    vector<int> assignedOrderIds;
    vector<string> assignedAddresses;
    vector<string> assignedItems;

    string optimizedRoute;
    string deliverySequence;
    int totalDistance;
};