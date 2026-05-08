#pragma once
#include <vector>
#include "../models/Driver.h"
#include "../models/Order.h"
#include "../ds/Graph.h"

using namespace std;

class DriverService {
private:
    vector<Driver> drivers;
    int nextDriverId;

    int findDriverIndex(int driverId);

public:
    DriverService();

    void initDrivers();

    bool driverExists(int driverId);
    Driver getDriver(int driverId);

    int findBestAvailableDriver(Order order, Graph& graph);
    bool assignOrdersToDriver(int driverId, vector<Order>& orders, Graph& graph);

    bool completeDelivery(int driverId);

    int totalDrivers();
    int availableDrivers();
};