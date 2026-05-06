#pragma once
#include <unordered_map>
#include "../models/Driver.h"
#include "../models/Order.h"

using namespace std;

class DriverService {
private:
    unordered_map<int, Driver> drivers;
    int nextDriverId;

public:
    DriverService();

    void initDrivers();
    bool driverExists(int driverId);
    Driver getDriver(int driverId);
    bool assignOrder(Order& order);
    bool completeDelivery(int driverId);
    int totalDrivers();
    int availableDrivers();
};