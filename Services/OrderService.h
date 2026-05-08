#pragma once
#include <vector>
#include "../models/Order.h"
#include "../ds/Cart.h"
#include "../ds/OrderQueue.h"
#include "../ds/OrderHistory.h"
#include "../ds/Analytics.h"
#include "../ds/Graph.h"
#include "DriverService.h"

class OrderService {
private:
    int nextOrderId;

    CartManager& cartManager;
    OrderQueue& orderQueue;
    OrderHistory& orderHistory;
    Analytics& analytics;
    Graph& graph;
    DriverService& driverService;

    vector<Order> lastAssignedBatch;

public:
    OrderService(
        CartManager& cart,
        OrderQueue& queue,
        OrderHistory& history,
        Analytics& analyticsRef,
        Graph& graphRef,
        DriverService& driverRef
    );

   Order checkout(string sessionId, string nearestLocation, int userDistance, bool isVIP);

    Order processNextOrder();

    Order undoLastOrder();

    vector<Order> getLastAssignedBatch();
};