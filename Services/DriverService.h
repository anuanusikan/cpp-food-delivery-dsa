#pragma once
#include <vector>
#include <string>
#include <climits>
#include "../models/Driver.h"
#include "../models/Order.h"
#include "../ds/Graph.h"

using namespace std;

class DriverService {
private:
    vector<Driver> drivers;
    int nextDriverId;

    int findDriverIndex(int driverId) {
        for (int i = 0; i < (int)drivers.size(); i++) {
            if (drivers[i].driverId == driverId) {
                return i;
            }
        }

        return -1;
    }

public:
    DriverService() {
        nextDriverId = 101;
    }

    void initDrivers() {
        drivers.clear();

        Driver d1;
        d1.driverId = 101;
        d1.name = "Driver_1";
        d1.status = AVAILABLE;
        d1.currentLocation = "Galle_Restaurant";
        d1.activeOrderCount = 0;
        d1.completedDeliveries = 0;
        d1.totalDistance = 0;
        drivers.push_back(d1);

        Driver d2;
        d2.driverId = 102;
        d2.name = "Driver_2";
        d2.status = AVAILABLE;
        d2.currentLocation = "Kalegana";
        d2.activeOrderCount = 0;
        d2.completedDeliveries = 0;
        d2.totalDistance = 0;
        drivers.push_back(d2);

        Driver d3;
        d3.driverId = 103;
        d3.name = "Driver_3";
        d3.status = AVAILABLE;
        d3.currentLocation = "Galle_Hospital";
        d3.activeOrderCount = 0;
        d3.completedDeliveries = 0;
        d3.totalDistance = 0;
        drivers.push_back(d3);
    }

    bool driverExists(int driverId) {
        return findDriverIndex(driverId) != -1;
    }

    Driver getDriver(int driverId) {
        int index = findDriverIndex(driverId);

        if (index == -1) {
            Driver emptyDriver;
            emptyDriver.driverId = -1;
            emptyDriver.name = "";
            emptyDriver.status = AVAILABLE;
            emptyDriver.currentLocation = "";
            emptyDriver.activeOrderCount = 0;
            emptyDriver.completedDeliveries = 0;
            emptyDriver.totalDistance = 0;
            return emptyDriver;
        }

        return drivers[index];
    }

    int findBestAvailableDriver(Order order, Graph& graph) {
        int bestDriverId = -1;
        int bestScore = INT_MAX;

        for (Driver& driver : drivers) {
            if (driver.status == AVAILABLE) {
                int distance = graph.distanceBetween(driver.currentLocation, order.address);

                int workloadPenalty = driver.activeOrderCount * 2;
                int score = distance + workloadPenalty;

                if (score < bestScore) {
                    bestScore = score;
                    bestDriverId = driver.driverId;
                }
            }
        }

        return bestDriverId;
    }

    bool assignOrdersToDriver(int driverId, vector<Order>& orders, Graph& graph) {
        int index = findDriverIndex(driverId);

        if (index == -1 || orders.empty()) {
            return false;
        }

        Driver& driver = drivers[index];

        if (driver.status != AVAILABLE) {
            return false;
        }

        driver.status = ON_DELIVERY;
        driver.assignedOrderIds.clear();
        driver.assignedAddresses.clear();
        driver.assignedItems.clear();

        vector<string> deliveryLocations;
        string sequence = "";

        for (int i = 0; i < (int)orders.size(); i++) {
            orders[i].assignedDriverId = driver.driverId;
            orders[i].status = IN_DELIVERY;

            driver.assignedOrderIds.push_back(orders[i].id);
            driver.assignedAddresses.push_back(orders[i].address);
            driver.assignedItems.push_back(orders[i].items);

            deliveryLocations.push_back(orders[i].address);

            sequence += "#" + to_string(orders[i].id) + " -> " + orders[i].address;

            if (i < (int)orders.size() - 1) {
                sequence += ", ";
            }
        }

        string fullRoute = driver.currentLocation;
        int fullDistance = 0;
        string currentLocation = driver.currentLocation;

        vector<bool> delivered(deliveryLocations.size(), false);
        int deliveredCount = 0;

        while (deliveredCount < (int)deliveryLocations.size()) {
            int bestIndex = -1;
            int bestDistance = INT_MAX;

            for (int i = 0; i < (int)deliveryLocations.size(); i++) {
                if (!delivered[i]) {
                    int distance = graph.distanceBetween(currentLocation, deliveryLocations[i]);

                    if (distance < bestDistance) {
                        bestDistance = distance;
                        bestIndex = i;
                    }
                }
            }

            if (bestIndex == -1) {
                break;
            }

            pair<int, string> routePart = graph.shortestPath(currentLocation, deliveryLocations[bestIndex]);

            if (routePart.first < 0) {
                break;
            }

            fullDistance += routePart.first;

            string partPath = routePart.second;
            size_t arrowPosition = partPath.find(" -> ");

            if (arrowPosition != string::npos) {
                partPath = partPath.substr(arrowPosition + 4);
            }

            fullRoute += " -> " + partPath;

            currentLocation = deliveryLocations[bestIndex];
            delivered[bestIndex] = true;
            deliveredCount++;
        }

        driver.activeOrderCount = orders.size();
        driver.optimizedRoute = fullRoute;
        driver.totalDistance = fullDistance;
        driver.deliverySequence = sequence;

        return true;
    }

    bool completeDelivery(int driverId) {
        int index = findDriverIndex(driverId);

        if (index == -1) {
            return false;
        }

        Driver& driver = drivers[index];

        if (driver.status != ON_DELIVERY) {
            return false;
        }

        if (!driver.assignedAddresses.empty()) {
            driver.currentLocation = driver.assignedAddresses[driver.assignedAddresses.size() - 1];
        }

        driver.completedDeliveries += driver.activeOrderCount;

        driver.status = AVAILABLE;
        driver.activeOrderCount = 0;

        driver.assignedOrderIds.clear();
        driver.assignedAddresses.clear();
        driver.assignedItems.clear();

        driver.optimizedRoute = "";
        driver.deliverySequence = "";
        driver.totalDistance = 0;

        return true;
    }

    int totalDrivers() {
        return drivers.size();
    }

    int availableDrivers() {
        int count = 0;

        for (Driver driver : drivers) {
            if (driver.status == AVAILABLE) {
                count++;
            }
        }

        return count;
    }
};