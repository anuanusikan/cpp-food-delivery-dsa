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

Order OrderService::checkout(string sessionId, string address, bool isVIP) {
    if (cartManager.isCartEmpty(sessionId)) {
        return Order{-1, "", "", false, PENDING, 0, -1};
    }

    string items = cartManager.getItemsAsString(sessionId);
    cartManager.clearCart(sessionId);

    analytics.countItems(items);
    graph.addAddress(address);

    Order order;
    order.id = nextOrderId++;
    order.items = items;
    order.address = address;
    order.isVIP = isVIP;
    order.status = PENDING;
    order.timestamp = chrono::system_clock::now().time_since_epoch().count();
    order.assignedDriverId = -1;

    orderQueue.push(order);

    return order;
}

Order OrderService::processNextOrder() {
    if (orderQueue.empty()) {
        return Order{-1, "", "", false, PENDING, 0, -1};
    }

    Order order = orderQueue.pop();
    order.status = READY;

    driverService.assignOrder(order);

    orderHistory.push(order);

    return order;
}

Order OrderService::undoLastOrder() {
    if (orderHistory.empty()) {
        return Order{-1, "", "", false, PENDING, 0, -1};
    }

    Order order = orderHistory.undo();
    order.status = PENDING;
    order.assignedDriverId = -1;

    orderQueue.push(order);

    return order;
}