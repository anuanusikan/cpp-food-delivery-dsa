#include "httplib.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>

#include "models/FoodItem.h"
#include "models/Order.h"
#include "models/Driver.h"

#include "ds/MenuManager.h"
#include "ds/Cart.h"
#include "ds/OrderQueue.h"
#include "ds/OrderHistory.h"
#include "ds/Analytics.h"
#include "ds/Graph.h"

#include "services/AuthService.h"
#include "services/DriverService.h"
#include "services/OrderService.h"

using namespace std;

string jsonEscape(const string& input) {
    string output;

    for (char c : input) {
        if (c == '"') {
            output += "\\\"";
        } else if (c == '\\') {
            output += "\\\\";
        } else if (c == '\n') {
            output += "\\n";
        } else {
            output += c;
        }
    }

    return output;
}

string statusToString(OrderStatus status) {
    switch (status) {
        case PENDING: return "PENDING";
        case PREPARING: return "PREPARING";
        case READY: return "READY";
        case ASSIGNED: return "ASSIGNED";
        case IN_DELIVERY: return "IN_DELIVERY";
        case DELIVERED: return "DELIVERED";
        case CANCELLED: return "CANCELLED";
        default: return "UNKNOWN";
    }
}

int main() {
    srand(time(nullptr));

    AuthService authService;

    MenuManager menuManager;
    menuManager.seedDefaultMenu();

    CartManager cartManager;
    OrderQueue orderQueue;
    OrderHistory orderHistory;
    Analytics analytics;

    Graph graph;
    graph.init();

    DriverService driverService;
    driverService.initDrivers();

    OrderService orderService(
        cartManager,
        orderQueue,
        orderHistory,
        analytics,
        graph,
        driverService
    );

    httplib::Server svr;

    svr.set_mount_point("/", "./public");

    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_redirect("/login.html");
    });

    // ================= AUTH =================

    svr.Get("/api/register", [&](const httplib::Request& req, httplib::Response& res) {
        string username = req.get_param_value("username");
        string password = req.get_param_value("password");

        string status = authService.registerUser(username, password);

        res.set_content("{\"status\":\"" + status + "\"}", "application/json");
    });

    svr.Get("/api/login", [&](const httplib::Request& req, httplib::Response& res) {
        string username = req.get_param_value("username");
        string password = req.get_param_value("password");

        string session = authService.login(username, password);

        if (!session.empty()) {
            res.set_content(
                "{\"status\":\"success\",\"session\":\"" + jsonEscape(session) + "\"}",
                "application/json"
            );
        } else {
            res.set_content("{\"status\":\"fail\"}", "application/json");
        }
    });

    svr.Get("/api/check", [&](const httplib::Request& req, httplib::Response& res) {
        string session = req.get_param_value("session");

        if (authService.checkSession(session)) {
            string username = authService.getUsername(session);

            res.set_content(
                "{\"status\":\"valid\",\"user\":\"" + jsonEscape(username) + "\"}",
                "application/json"
            );
        } else {
            res.set_content("{\"status\":\"invalid\"}", "application/json");
        }
    });

    svr.Get("/api/logout", [&](const httplib::Request& req, httplib::Response& res) {
        string session = req.get_param_value("session");
        authService.logout(session);

        res.set_content("{\"status\":\"logged_out\"}", "application/json");
    });

    // ================= MENU =================

    svr.Get("/api/menu", [&](const httplib::Request& req, httplib::Response& res) {
        string q = "";

        if (req.has_param("q")) {
            q = req.get_param_value("q");
        }

        auto menu = menuManager.searchMenu(q);

        string json = "{";
        bool firstCategory = true;

        for (auto& categoryPair : menu) {
            if (!firstCategory) {
                json += ",";
            }

            json += "\"" + jsonEscape(categoryPair.first) + "\":[";

            bool firstItem = true;

            for (auto& item : categoryPair.second) {
                if (!firstItem) {
                    json += ",";
                }

                json += "{";
                json += "\"id\":" + to_string(item.id) + ",";
                json += "\"name\":\"" + jsonEscape(item.name) + "\",";
                json += "\"price\":" + to_string(item.price) + ",";
                json += "\"category\":\"" + jsonEscape(item.category) + "\"";
                json += "}";

                firstItem = false;
            }

            json += "]";
            firstCategory = false;
        }

        json += "}";

        res.set_content(json, "application/json");
    });

    svr.Get("/api/menu/add", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            int id = stoi(req.get_param_value("id"));
            string name = req.get_param_value("name");
            int price = stoi(req.get_param_value("price"));
            string category = req.get_param_value("category");

            FoodItem item{id, name, price, category};
            menuManager.addItem(item);

            res.set_content("{\"status\":\"success\"}", "application/json");
        } catch (...) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    // ================= CART =================

    svr.Get("/api/cart/add", [&](const httplib::Request& req, httplib::Response& res) {
        string session = req.get_param_value("sessionId");

        if (!authService.checkSession(session)) {
            res.set_content("{\"status\":\"unauthorized\"}", "application/json");
            return;
        }

        try {
            string item = req.get_param_value("item");
            int price = stoi(req.get_param_value("price"));

            cartManager.addItem(session, item, price);

            res.set_content("{\"status\":\"added\"}", "application/json");
        } catch (...) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    svr.Get("/api/cart/view", [&](const httplib::Request& req, httplib::Response& res) {
        string session = req.get_param_value("sessionId");

        if (!authService.checkSession(session)) {
            res.set_content("[]", "application/json");
            return;
        }

        auto items = cartManager.getCartItems(session);

        string json = "[";
        bool first = true;

        for (auto& item : items) {
            if (!first) {
                json += ",";
            }

            json += "{";
            json += "\"item\":\"" + jsonEscape(item.first) + "\",";
            json += "\"price\":" + to_string(item.second);
            json += "}";

            first = false;
        }

        json += "]";

        res.set_content(json, "application/json");
    });

    // ================= CHECKOUT WITH DYNAMIC USER LOCATION =================

    svr.Get("/api/checkout", [&](const httplib::Request& req, httplib::Response& res) {
        string session = req.get_param_value("sessionId");

        if (!authService.checkSession(session)) {
            res.set_content("{\"status\":\"unauthorized\"}", "application/json");
            return;
        }

        try {
            string nearestLocation = req.get_param_value("nearestLocation");
            int userDistance = stoi(req.get_param_value("userDistance"));
            bool vip = req.get_param_value("vip") == "true";

            Order order = orderService.checkout(session, nearestLocation, userDistance, vip);

            if (order.id == -1) {
                res.set_content("{\"status\":\"empty_cart\"}", "application/json");
                return;
            }

            auto routeResult = graph.shortestPath("Galle_Restaurant", order.address);

            string json = "{";
            json += "\"status\":\"success\",";
            json += "\"order_id\":" + to_string(order.id) + ",";
            json += "\"deliveryNode\":\"" + jsonEscape(order.address) + "\",";
            json += "\"nearestLocation\":\"" + jsonEscape(order.nearestLocation) + "\",";
            json += "\"userDistance\":" + to_string(order.userDistance) + ",";
            json += "\"totalDistance\":" + to_string(routeResult.first) + ",";
            json += "\"shortestPath\":\"" + jsonEscape(routeResult.second) + "\",";
            json += "\"vip\":" + string(order.isVIP ? "true" : "false");
            json += "}";

            res.set_content(json, "application/json");
        } catch (...) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    // ================= USER ORDERS / CANCEL =================

    svr.Get("/api/user/orders", [&](const httplib::Request& req, httplib::Response& res) {
        string session = req.get_param_value("sessionId");

        if (!authService.checkSession(session)) {
            res.set_content("[]", "application/json");
            return;
        }

        vector<Order> orders = orderService.getUserPendingOrders(session);

        string json = "[";
        bool first = true;

        for (Order& order : orders) {
            if (!first) {
                json += ",";
            }

            auto routeResult = graph.shortestPath("Galle_Restaurant", order.address);

            json += "{";
            json += "\"id\":" + to_string(order.id) + ",";
            json += "\"items\":\"" + jsonEscape(order.items) + "\",";
            json += "\"address\":\"" + jsonEscape(order.address) + "\",";
            json += "\"nearestLocation\":\"" + jsonEscape(order.nearestLocation) + "\",";
            json += "\"userDistance\":" + to_string(order.userDistance) + ",";
            json += "\"distanceFromRestaurant\":" + to_string(routeResult.first) + ",";
            json += "\"status\":\"" + statusToString(order.status) + "\",";
            json += "\"vip\":" + string(order.isVIP ? "true" : "false");
            json += "}";

            first = false;
        }

        json += "]";

        res.set_content(json, "application/json");
    });

    svr.Get("/api/user/cancel", [&](const httplib::Request& req, httplib::Response& res) {
        string session = req.get_param_value("sessionId");

        if (!authService.checkSession(session)) {
            res.set_content("{\"status\":\"unauthorized\"}", "application/json");
            return;
        }

        try {
            int orderId = stoi(req.get_param_value("orderId"));

            bool cancelled = orderService.cancelPendingOrder(orderId, session);

            if (cancelled) {
                res.set_content("{\"status\":\"cancelled\"}", "application/json");
            } else {
                res.set_content(
                    "{\"status\":\"not_allowed\",\"message\":\"Order cannot be cancelled because it may already be processed or does not belong to this user.\"}",
                    "application/json"
                );
            }
        } catch (...) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    // ================= ADMIN =================

    svr.Get("/api/admin/queue", [&](const httplib::Request& req, httplib::Response& res) {
        auto orders = orderQueue.getAll();

        string json = "[";
        bool first = true;

        for (auto& order : orders) {
            auto routeResult = graph.shortestPath("Galle_Restaurant", order.address);

            if (!first) {
                json += ",";
            }

            json += "{";
            json += "\"id\":" + to_string(order.id) + ",";
            json += "\"items\":\"" + jsonEscape(order.items) + "\",";
            json += "\"address\":\"" + jsonEscape(order.address) + "\",";
            json += "\"nearestLocation\":\"" + jsonEscape(order.nearestLocation) + "\",";
            json += "\"userDistance\":" + to_string(order.userDistance) + ",";
            json += "\"distanceFromRestaurant\":" + to_string(routeResult.first) + ",";
            json += "\"shortestPath\":\"" + jsonEscape(routeResult.second) + "\",";
            json += "\"status\":\"" + statusToString(order.status) + "\",";
            json += "\"vip\":" + string(order.isVIP ? "true" : "false");
            json += "}";

            first = false;
        }

        json += "]";

        res.set_content(json, "application/json");
    });

    svr.Get("/api/admin/process", [&](const httplib::Request& req, httplib::Response& res) {
        Order order = orderService.processNextOrder();

        if (order.id == -1) {
            res.set_content("{\"status\":\"empty\"}", "application/json");
            return;
        }

        if (order.id == -2) {
            res.set_content(
                "{\"status\":\"waiting\",\"message\":\"No available drivers. Order kept in queue.\"}",
                "application/json"
            );
            return;
        }

        vector<Order> batch = orderService.getLastAssignedBatch();

        string orderIds = "";
        string orderLocations = "";

        for (int i = 0; i < (int)batch.size(); i++) {
            orderIds += "#" + to_string(batch[i].id);
            orderLocations += batch[i].address;

            if (i < (int)batch.size() - 1) {
                orderIds += ", ";
                orderLocations += " -> ";
            }
        }

        string json = "{";
        json += "\"status\":\"success\",";
        json += "\"order_id\":" + to_string(order.id) + ",";
        json += "\"assignedDriverId\":" + to_string(order.assignedDriverId) + ",";
        json += "\"batchCount\":" + to_string(batch.size()) + ",";
        json += "\"orderIds\":\"" + jsonEscape(orderIds) + "\",";
        json += "\"locations\":\"" + jsonEscape(orderLocations) + "\"";
        json += "}";

        res.set_content(json, "application/json");
    });

    svr.Get("/api/admin/undo", [&](const httplib::Request& req, httplib::Response& res) {
        Order order = orderService.undoLastOrder();

        if (order.id == -1) {
            res.set_content("{\"status\":\"no_history\"}", "application/json");
            return;
        }

        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    // ================= DRIVER =================

    svr.Get("/api/driver/login", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            int driverId = stoi(req.get_param_value("driverId"));

            if (!driverService.driverExists(driverId)) {
                res.set_content("{\"status\":\"error\"}", "application/json");
                return;
            }

            Driver driver = driverService.getDriver(driverId);

            string json = "{";
            json += "\"status\":\"success\",";
            json += "\"name\":\"" + jsonEscape(driver.name) + "\",";
            json += "\"driverId\":" + to_string(driver.driverId);
            json += "}";

            res.set_content(json, "application/json");
        } catch (...) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    svr.Get("/api/driver/next-assignment", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            int driverId = stoi(req.get_param_value("driverId"));

            if (!driverService.driverExists(driverId)) {
                res.set_content("{}", "application/json");
                return;
            }

            Driver driver = driverService.getDriver(driverId);

            if (driver.activeOrderCount == 0) {
                res.set_content("{}", "application/json");
                return;
            }

            string ids = "";
            string items = "";
            string addresses = "";

            for (int i = 0; i < (int)driver.assignedOrderIds.size(); i++) {
                ids += "#" + to_string(driver.assignedOrderIds[i]);
                items += driver.assignedItems[i];
                addresses += driver.assignedAddresses[i];

                if (i < (int)driver.assignedOrderIds.size() - 1) {
                    ids += ", ";
                    items += " | ";
                    addresses += " -> ";
                }
            }

            string json = "{";
            json += "\"id\":" + to_string(driver.assignedOrderIds[0]) + ",";
            json += "\"orderIds\":\"" + jsonEscape(ids) + "\",";
            json += "\"items\":\"" + jsonEscape(items) + "\",";
            json += "\"address\":\"" + jsonEscape(addresses) + "\",";
            json += "\"status\":\"IN_DELIVERY\",";
            json += "\"driverLocation\":\"" + jsonEscape(driver.currentLocation) + "\",";
            json += "\"activeOrderCount\":" + to_string(driver.activeOrderCount) + ",";
            json += "\"deliverySequence\":\"" + jsonEscape(driver.deliverySequence) + "\",";
            json += "\"optimizedRoute\":\"" + jsonEscape(driver.optimizedRoute) + "\",";
            json += "\"totalDistance\":" + to_string(driver.totalDistance) + ",";
            json += "\"completedDeliveries\":" + to_string(driver.completedDeliveries);
            json += "}";

            res.set_content(json, "application/json");
        } catch (...) {
            res.set_content("{}", "application/json");
        }
    });

    svr.Get("/api/driver/complete", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            int driverId = stoi(req.get_param_value("driverId"));

            bool ok = driverService.completeDelivery(driverId);

            if (ok) {
                res.set_content("{\"status\":\"success\"}", "application/json");
            } else {
                res.set_content("{\"status\":\"error\"}", "application/json");
            }
        } catch (...) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    // ================= ROUTE =================

    svr.Get("/api/route", [&](const httplib::Request& req, httplib::Response& res) {
        string dest = req.get_param_value("dest");

        auto result = graph.shortestPath("Galle_Restaurant", dest);

        string json = "{";
        json += "\"distance\":" + to_string(result.first) + ",";
        json += "\"path\":\"" + jsonEscape(result.second) + "\"";
        json += "}";

        res.set_content(json, "application/json");
    });

    // ================= ANALYTICS =================

    svr.Get("/api/analytics/popular", [&](const httplib::Request& req, httplib::Response& res) {
        int limit = 5;

        if (req.has_param("limit")) {
            limit = stoi(req.get_param_value("limit"));
        }

        auto topItems = analytics.getTop(limit);

        string json = "[";
        bool first = true;

        for (auto& item : topItems) {
            if (!first) {
                json += ",";
            }

            json += "{";
            json += "\"item\":\"" + jsonEscape(item.first) + "\",";
            json += "\"count\":" + to_string(item.second);
            json += "}";

            first = false;
        }

        json += "]";

        res.set_content(json, "application/json");
    });

    svr.Get("/api/analytics/stats", [&](const httplib::Request& req, httplib::Response& res) {
        string json = "{";
        json += "\"queue_size\":" + to_string(orderQueue.size()) + ",";
        json += "\"active_sessions\":" + to_string(cartManager.getActiveSessionCount()) + ",";
        json += "\"total_drivers\":" + to_string(driverService.totalDrivers()) + ",";
        json += "\"available_drivers\":" + to_string(driverService.availableDrivers()) + ",";
        json += "\"history_size\":" + to_string(orderHistory.size()) + ",";
        json += "\"cart_items\":" + to_string(cartManager.getTotalCartItems()) + ",";
        json += "\"unique_items_ordered\":" + to_string(analytics.uniqueItemCount()) + ",";
        json += "\"graph_nodes\":" + to_string(graph.nodeCount()) + ",";
        json += "\"menu_items\":" + to_string(menuManager.totalItems());
        json += "}";

        res.set_content(json, "application/json");
    });

    cout << "====================================================\n";
    cout << " Smart Food Delivery System Running\n";
    cout << " URL: http://localhost:8080/login.html\n";
    cout << " Graph Start Node: Galle_Restaurant\n";
    cout << " DSA: Manual HashMap, LinkedList Cart, Manual Heap,\n";
    cout << "      Stack, Trie, Graph, Dijkstra, Smart Delivery\n";
    cout << "====================================================\n";

    svr.listen("localhost", 8080);

    return 0;
}