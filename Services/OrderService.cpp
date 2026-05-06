#include "OrderService.h"
#include <iostream>
#include <chrono>

using namespace std;

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

Order OrderService::checkout(string sessionId, string address, bool isVIP) {
    if (cartManager.isCartEmpty(sessionId)) {
        cout << "[Order] Checkout failed - cart is empty\n";
        return Order{-1, "", "", false, PENDING, 0, -1};
    }

    string items = cartManager.getItemsAsString(sessionId);
    cartManager.clearCart(sessionId);

    // track what was ordered for analytics
    analytics.countItems(items);

    Order order;
    order.id = nextOrderId++;
    order.items = items;
    order.address = address;
    order.isVIP = isVIP;
    order.status = PENDING;
    order.timestamp = chrono::system_clock::now().time_since_epoch().count();
    order.assignedDriverId = -1;

    orderQueue.push(order);

    cout << "[Order] New order #" << order.id << " placed";
    if (isVIP) cout << " [VIP]";
    cout << " -> " << address << "\n";
    cout << "[Order] Items: " << items << "\n";

    // TODO: maybe add email notification here later

    return order;
}

Order OrderService::processNextOrder() {
    if (orderQueue.empty()) {
        cout << "[Order] Queue is empty, nothing to process\n";
        return Order{-1, "", "", false, PENDING, 0, -1};
    }

    Order order = orderQueue.pop();
    order.status = READY;

    cout << "[Order] Processing order #" << order.id << "\n";

    // try to assign a driver
    bool assigned = driverService.assignOrder(order);
    if (!assigned) {
        cout << "[Order] Warning: no available drivers for order #" << order.id << "\n";
    }

    orderHistory.push(order);

    return order;
}

Order OrderService::undoLastOrder() {
    if (orderHistory.empty()) {
        cout << "[Order] Nothing to undo\n";
        return Order{-1, "", "", false, PENDING, 0, -1};
    }

    Order order = orderHistory.undo();
    order.status = PENDING;
    order.assignedDriverId = -1;

    orderQueue.push(order);

    cout << "[Order] Order #" << order.id << " restored to queue\n";
    return order;
}