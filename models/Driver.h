#pragma once
#include <string>

using namespace std;

enum DriverStatus {
    AVAILABLE,
    ON_DELIVERY
};

struct Driver {
    int driverId;
    string name;
    DriverStatus status;
    int currentOrderId;
    string currentAddress;
    string currentItems;
    int completedDeliveries;
};