#include "OrderService.h"
#include <chrono>

OrderService::OrderService(
    CartManager& cart,
    OrderQueue& queue,
    OrderHistory& history,
    Analytics& analyticsRef,
    Graph& graphRef,
    DriverService& driverRef
) : cartManager(cart),
    orderQueue(queue),
    orderHistory(history),
    analytics(analyticsRef),
    graph(graphRef),
    driverService(driverRef) {
    nextOrderId = 1000;
}

Order OrderService::checkout(string sessionId, string nearestLocation, int userDistance, bool isVIP) {
    if (cartManager.isCartEmpty(sessionId)) {
        return Order{-1, "", "", "", 0, false, PENDING, 0, -1, 0};
    }

    string items = cartManager.getItemsAsString(sessionId);
    cartManager.clearCart(sessionId);

    analytics.countItems(items);

    int newOrderId = nextOrderId++;

    string customerNode = "Customer_" + to_string(newOrderId);

    graph.addDynamicUserLocation(customerNode, nearestLocation, userDistance);

    Order order;
    order.id = newOrderId;
    order.items = items;
    order.address = customerNode;
    order.nearestLocation = nearestLocation;
    order.userDistance = userDistance;
    order.isVIP = isVIP;
    order.status = PENDING;
    order.timestamp = chrono::system_clock::now().time_since_epoch().count();
    order.assignedDriverId = -1;
    order.distanceFromRestaurant = graph.distanceBetween("Galle_Restaurant", customerNode);

    orderQueue.push(order);

    return order;
}

Order OrderService::processNextOrder() {
    lastAssignedBatch.clear();

    if (orderQueue.empty()) {
        return Order{-1, "", "", "", 0, false, PENDING, 0, -1, 0};
    }

    Order mainOrder = orderQueue.pop();
    mainOrder.status = READY;

    int selectedDriverId = driverService.findBestAvailableDriver(mainOrder, graph);

    if (selectedDriverId == -1) {
        mainOrder.status = PENDING;
        mainOrder.assignedDriverId = -1;
        orderQueue.push(mainOrder);

        return Order{-2, "", "", "", 0, false, PENDING, 0, -1, 0};
    }

    vector<Order> batch;
    batch.push_back(mainOrder);

    vector<Order> waitingOrders = orderQueue.getAll();

    for (Order candidate : waitingOrders) {
        // Maximum 2 orders per driver trip
        if ((int)batch.size() >= 2) {
            break;
        }

        int distanceBetweenOrders = graph.distanceBetween(mainOrder.address, candidate.address);

        bool sameNearestLocation = mainOrder.nearestLocation == candidate.nearestLocation;
        bool nearbyDistance = distanceBetweenOrders <= 5;

        bool canBatch = sameNearestLocation || nearbyDistance;

        if (canBatch) {
            bool removed = orderQueue.removeById(candidate.id);

            if (removed) {
                candidate.status = READY;
                batch.push_back(candidate);
            }
        }
    }

    bool assigned = driverService.assignOrdersToDriver(selectedDriverId, batch, graph);

    if (!assigned) {
        for (Order order : batch) {
            order.status = PENDING;
            order.assignedDriverId = -1;
            orderQueue.push(order);
        }

        return Order{-2, "", "", "", 0, false, PENDING, 0, -1, 0};
    }

    for (Order order : batch) {
        orderHistory.push(order);
    }

    lastAssignedBatch = batch;

    return batch[0];
}

Order OrderService::undoLastOrder() {
    if (orderHistory.empty()) {
        return Order{-1, "", "", "", 0, false, PENDING, 0, -1, 0};
    }

    Order order = orderHistory.undo();

    order.status = PENDING;
    order.assignedDriverId = -1;

    orderQueue.push(order);

    return order;
}

vector<Order> OrderService::getLastAssignedBatch() {
    return lastAssignedBatch;
}