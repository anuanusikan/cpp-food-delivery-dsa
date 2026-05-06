#include "DriverService.h"

DriverService::DriverService() {
    nextDriverId = 101;
}

void DriverService::initDrivers() {
    for (int i = 0; i < 3; i++) {
        Driver d;
        d.driverId = nextDriverId++;
        d.name = "Driver_" + to_string(i + 1);
        d.status = AVAILABLE;
        d.currentOrderId = -1;
        d.currentAddress = "";
        d.currentItems = "";
        d.completedDeliveries = 0;

        drivers[d.driverId] = d;
    }
}

bool DriverService::driverExists(int driverId) {
    return drivers.find(driverId) != drivers.end();
}

Driver DriverService::getDriver(int driverId) {
    if (!driverExists(driverId)) {
        return Driver{-1, "", AVAILABLE, -1, "", "", 0};
    }

    return drivers[driverId];
}

bool DriverService::assignOrder(Order& order) {
    for (auto& pair : drivers) {
        Driver& driver = pair.second;

        if (driver.status == AVAILABLE) {
            driver.status = ON_DELIVERY;
            driver.currentOrderId = order.id;
            driver.currentAddress = order.address;
            driver.currentItems = order.items;

            order.assignedDriverId = driver.driverId;
            order.status = IN_DELIVERY;

            return true;
        }
    }

    return false;
}

bool DriverService::completeDelivery(int driverId) {
    if (!driverExists(driverId)) {
        return false;
    }

    Driver& driver = drivers[driverId];

    if (driver.currentOrderId == -1) {
        return false;
    }

    driver.status = AVAILABLE;
    driver.currentOrderId = -1;
    driver.currentAddress = "";
    driver.currentItems = "";
    driver.completedDeliveries++;

    return true;
}

int DriverService::totalDrivers() {
    return drivers.size();
}

int DriverService::availableDrivers() {
    int count = 0;

    for (auto& pair : drivers) {
        if (pair.second.status == AVAILABLE) {
            count++;
        }
    }

    return count;
}