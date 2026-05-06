#include "httplib.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <limits>

#include "models/FoodItem.h"
#include "models/Order.h"
#include "models/Driver.h"

#include "ds/MenuManager.h"
#include "ds/Cart.h"
#include "ds/OrderQueue.h"
#include "ds/OrderHistory.h"
#include "ds/Analytics.h"
#include "ds/Graph.h"

#include "Services/AuthService.h"
#include "Services/DriverService.h"
#include "Services/OrderService.h"

using namespace std;

// ---- helper: escape special chars for JSON output ----
string jsonEscape(const string& s) {
    string out;
    for (char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else                out += c;
    }
    return out;
}

string statusToStr(OrderStatus s) {
    switch (s) {
        case PENDING:     return "PENDING";
        case PREPARING:   return "PREPARING";
        case READY:       return "READY";
        case ASSIGNED:    return "ASSIGNED";
        case IN_DELIVERY: return "IN_DELIVERY";
        case DELIVERED:   return "DELIVERED";
        default:          return "UNKNOWN";
    }
}

// ======================================================
//  CONSOLE MODE - runs before web server
// ======================================================

void printMenu(MenuManager& menu) {
    cout << "\n--- MENU ---\n";
    auto cats = menu.getMenu();
    for (auto& cat : cats) {
        cout << "\n[" << cat.first << "]\n";
        for (auto& item : cat.second) {
            cout << "  " << item.id << ". " << item.name << " - Rs." << item.price << "\n";
        }
    }
}

void consolePrintCart(CartManager& cart, const string& session) {
    auto items = cart.getCartItems(session);
    if (items.empty()) {
        cout << "  (cart is empty)\n";
        return;
    }
    int total = 0;
    int i = 1;
    for (auto& item : items) {
        cout << "  " << i++ << ". " << item.first << " - Rs." << item.second << "\n";
        total += item.second;
    }
    cout << "  Total: Rs." << total << "\n";
}

void runConsoleMode(
    MenuManager& menuManager,
    CartManager& cartManager,
    OrderQueue& orderQueue,
    OrderHistory& orderHistory,
    Analytics& analytics,
    Graph& graph,
    AuthService& authService,
    DriverService& driverService,
    OrderService& orderService
) {
    cout << "\n========================================\n";
    cout << "   FOOD DELIVERY SYSTEM - CONSOLE MODE\n";
    cout << "   Galle Restaurant, Sri Lanka\n";
    cout << "   DSA: HashMap, Linked List, Trie,\n";
    cout << "        Priority Queue, Stack, Graph\n";
    cout << "========================================\n";

    string session = "";
    string username = "";

    // login or register first
    while (session.empty()) {
        cout << "\n1. Register\n2. Login\n3. Skip (guest)\nChoice: ";
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 3) {
            // guest session
            session = "guest_" + to_string(time(nullptr));
            username = "Guest";
            cout << "Continuing as Guest\n";
            break;
        }

        cout << "Username: ";
        getline(cin, username);
        cout << "Password: ";
        string password;
        getline(cin, password);

        if (choice == 1) {
            string result = authService.registerUser(username, password);
            if (result == "registered") {
                cout << "Registered! Now login.\n";
            } else {
                cout << "User already exists. Try login.\n";
            }
        } else if (choice == 2) {
            session = authService.login(username, password);
            if (session.empty()) {
                cout << "Wrong username or password.\n";
                username = "";
            } else {
                cout << "Welcome, " << username << "!\n";
            }
        }
    }

    // main console menu loop
    bool running = true;
    while (running) {
        cout << "\n--- MAIN MENU ---\n";
        cout << "1. View Menu\n";
        cout << "2. Search Menu\n";
        cout << "3. Add Item to Cart\n";
        cout << "4. Remove Item from Cart\n";
        cout << "5. View Cart\n";
        cout << "6. Checkout\n";
        cout << "7. Admin: View Order Queue\n";
        cout << "8. Admin: Process Next Order\n";
        cout << "9. Admin: Undo Last Order\n";
        cout << "10. View Popular Items\n";
        cout << "11. Calculate Route\n";
        cout << "0. Launch Web Server and Exit Console\n";
        cout << "Choice: ";

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) {
            running = false;

        } else if (choice == 1) {
            printMenu(menuManager);

        } else if (choice == 2) {
            cout << "Search: ";
            string q;
            getline(cin, q);
            auto results = menuManager.searchMenu(q);
            if (results.empty()) {
                cout << "No results found.\n";
            } else {
                for (auto& cat : results) {
                    cout << "\n[" << cat.first << "]\n";
                    for (auto& item : cat.second) {
                        cout << "  " << item.id << ". " << item.name << " - Rs." << item.price << "\n";
                    }
                }
            }

        } else if (choice == 3) {
            printMenu(menuManager);
            cout << "Enter item name to add: ";
            string itemName;
            getline(cin, itemName);
            cout << "Price: ";
            int price;
            cin >> price;
            cin.ignore();
            cartManager.addItem(session, itemName, price);
            cout << "Added to cart!\n";

        } else if (choice == 4) {
            cout << "\nYour Cart:\n";
            consolePrintCart(cartManager, session);
            cout << "Enter item name to remove: ";
            string itemName;
            getline(cin, itemName);
            bool removed = cartManager.removeItem(session, itemName);
            if (removed) {
                cout << "Item removed.\n";
            } else {
                cout << "Item not found in cart.\n";
            }

        } else if (choice == 5) {
            cout << "\nYour Cart:\n";
            consolePrintCart(cartManager, session);

        } else if (choice == 6) {
            if (cartManager.isCartEmpty(session)) {
                cout << "Cart is empty!\n";
            } else {
                // show available delivery zones
                auto zones = graph.getDeliveryZones();
                cout << "\nAvailable delivery zones:\n";
                for (int i = 0; i < (int)zones.size(); i++) {
                    cout << "  " << (i + 1) << ". " << zones[i] << "\n";
                }
                cout << "Enter zone number: ";
                int zoneChoice;
                cin >> zoneChoice;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                if (zoneChoice < 1 || zoneChoice > (int)zones.size()) {
                    cout << "Invalid zone.\n";
                } else {
                    string address = zones[zoneChoice - 1];
                    cout << "VIP order? (1=yes, 0=no): ";
                    int vip;
                    cin >> vip;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    Order o = orderService.checkout(session, address, vip == 1);
                    if (o.id == -1) {
                        cout << "Checkout failed.\n";
                    } else {
                        cout << "Order #" << o.id << " placed -> " << address << "\n";
                        if (vip == 1) cout << "(VIP - will be processed first)\n";
                    }
                }
            }

        } else if (choice == 7) {
            auto orders = orderQueue.getAll();
            if (orders.empty()) {
                cout << "Queue is empty.\n";
            } else {
                cout << "\n--- ORDER QUEUE (Priority: VIP first) ---\n";
                for (auto& o : orders) {
                    cout << "  Order #" << o.id;
                    if (o.isVIP) cout << " [VIP]";
                    cout << " | " << o.items << " | -> " << o.address << "\n";
                }
            }

        } else if (choice == 8) {
            Order o = orderService.processNextOrder();
            if (o.id == -1) {
                cout << "Queue is empty.\n";
            } else {
                cout << "Processed Order #" << o.id << "\n";
                if (o.assignedDriverId != -1) {
                    cout << "Assigned to Driver #" << o.assignedDriverId << "\n";
                } else {
                    cout << "No available drivers!\n";
                }
            }

        } else if (choice == 9) {
            Order o = orderService.undoLastOrder();
            if (o.id == -1) {
                cout << "Nothing to undo.\n";
            } else {
                cout << "Order #" << o.id << " moved back to queue.\n";
            }

        } else if (choice == 10) {
            auto top = analytics.getTop(5);
            if (top.empty()) {
                cout << "No orders placed yet.\n";
            } else {
                cout << "\n--- TOP ITEMS ---\n";
                int rank = 1;
                for (auto& item : top) {
                    cout << "  " << rank++ << ". " << item.first << " (" << item.second << " orders)\n";
                }
            }

        } else if (choice == 11) {
            auto zones = graph.getDeliveryZones();
            cout << "\nAvailable delivery zones:\n";
            for (int i = 0; i < (int)zones.size(); i++) {
                cout << "  " << (i + 1) << ". " << zones[i] << "\n";
            }
            cout << "Enter zone number: ";
            int zoneChoice;
            cin >> zoneChoice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (zoneChoice < 1 || zoneChoice > (int)zones.size()) {
                cout << "Invalid zone.\n";
            } else {
                string dest = zones[zoneChoice - 1];
                auto result = graph.shortestPath(dest);
                if (result.first == -1) {
                    cout << "Location not found in city map.\n";
                } else {
                    cout << "Dijkstra shortest path to " << dest << ":\n";
                    cout << "  Path: " << result.second << "\n";
                    cout << "  Distance: " << result.first << " km\n";
                }
            }

        } else {
            cout << "Invalid choice.\n";
        }
    }

    cout << "\nStarting web server...\n";
}

// ======================================================
//  MAIN
// ======================================================

int main() {
    srand(time(nullptr));

    // setup all DSA components
    AuthService authService;

    MenuManager menuManager;
    menuManager.seedDefaultMenu();

    CartManager cartManager;
    OrderQueue  orderQueue;
    OrderHistory orderHistory;
    Analytics   analytics;

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

    // run console mode first - user can interact before web server
    runConsoleMode(
        menuManager, cartManager, orderQueue, orderHistory,
        analytics, graph, authService, driverService, orderService
    );

    // start HTTP server
    httplib::Server svr;
    svr.set_mount_point("/", "./public");

    // ===== AUTH =====

    svr.Get("/api/register", [&](const httplib::Request& req, httplib::Response& res) {
        string username = req.get_param_value("username");
        string password = req.get_param_value("password");
        string status = authService.registerUser(username, password);
        res.set_content("{\"status\":\"" + status + "\"}", "application/json");
    });

    svr.Get("/api/login", [&](const httplib::Request& req, httplib::Response& res) {
        string username = req.get_param_value("username");
        string password = req.get_param_value("password");
        string session  = authService.login(username, password);

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
            string user = authService.getUsername(session);
            res.set_content(
                "{\"status\":\"valid\",\"user\":\"" + jsonEscape(user) + "\"}",
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

    // ===== MENU =====

    svr.Get("/api/menu", [&](const httplib::Request& req, httplib::Response& res) {
        string q = req.has_param("q") ? req.get_param_value("q") : "";
        auto menu = menuManager.searchMenu(q);

        string json = "{";
        bool firstCat = true;

        for (auto& cat : menu) {
            if (!firstCat) json += ",";
            json += "\"" + jsonEscape(cat.first) + "\":[";

            bool firstItem = true;
            for (auto& item : cat.second) {
                if (!firstItem) json += ",";
                json += "{";
                json += "\"id\":"    + to_string(item.id)    + ",";
                json += "\"name\":\"" + jsonEscape(item.name) + "\",";
                json += "\"price\":" + to_string(item.price)  + ",";
                json += "\"category\":\"" + jsonEscape(item.category) + "\"";
                json += "}";
                firstItem = false;
            }

            json += "]";
            firstCat = false;
        }

        json += "}";
        res.set_content(json, "application/json");
    });

    svr.Get("/api/menu/add", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            int    id       = stoi(req.get_param_value("id"));
            string name     = req.get_param_value("name");
            int    price    = stoi(req.get_param_value("price"));
            string category = req.get_param_value("category");

            FoodItem item{id, name, price, category};
            menuManager.addItem(item);

            res.set_content("{\"status\":\"success\"}", "application/json");
        } catch (...) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    // ===== CART =====

    svr.Get("/api/cart/add", [&](const httplib::Request& req, httplib::Response& res) {
        string session = req.get_param_value("sessionId");
        if (!authService.checkSession(session)) {
            res.set_content("{\"status\":\"unauthorized\"}", "application/json");
            return;
        }

        try {
            string item = req.get_param_value("item");
            int price   = stoi(req.get_param_value("price"));
            cartManager.addItem(session, item, price);
            res.set_content("{\"status\":\"added\"}", "application/json");
        } catch (...) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    svr.Get("/api/cart/remove", [&](const httplib::Request& req, httplib::Response& res) {
        string session = req.get_param_value("sessionId");
        if (!authService.checkSession(session)) {
            res.set_content("{\"status\":\"unauthorized\"}", "application/json");
            return;
        }

        string item = req.get_param_value("item");
        bool removed = cartManager.removeItem(session, item);

        if (removed) {
            res.set_content("{\"status\":\"removed\"}", "application/json");
        } else {
            res.set_content("{\"status\":\"not_found\"}", "application/json");
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
            if (!first) json += ",";
            json += "{\"item\":\"" + jsonEscape(item.first) + "\",\"price\":" + to_string(item.second) + "}";
            first = false;
        }

        json += "]";
        res.set_content(json, "application/json");
    });

    // ===== CHECKOUT =====

    svr.Get("/api/checkout", [&](const httplib::Request& req, httplib::Response& res) {
        string session = req.get_param_value("sessionId");
        if (!authService.checkSession(session)) {
            res.set_content("{\"status\":\"unauthorized\"}", "application/json");
            return;
        }

        string address = req.get_param_value("address");
        bool vip = req.get_param_value("vip") == "true";

        Order order = orderService.checkout(session, address, vip);

        if (order.id == -1) {
            res.set_content("{\"status\":\"empty_cart\"}", "application/json");
            return;
        }

        string json = "{";
        json += "\"status\":\"success\",";
        json += "\"order_id\":" + to_string(order.id) + ",";
        json += "\"vip\":" + string(order.isVIP ? "true" : "false");
        json += "}";
        res.set_content(json, "application/json");
    });

    // ===== ADMIN =====

    svr.Get("/api/admin/queue", [&](const httplib::Request& req, httplib::Response& res) {
        auto orders = orderQueue.getAll();
        string json = "[";
        bool first = true;

        for (auto& o : orders) {
            if (!first) json += ",";
            json += "{";
            json += "\"id\":"      + to_string(o.id)     + ",";
            json += "\"items\":\"" + jsonEscape(o.items)  + "\",";
            json += "\"address\":\"" + jsonEscape(o.address) + "\",";
            json += "\"status\":\"" + statusToStr(o.status) + "\",";
            json += "\"vip\":"     + string(o.isVIP ? "true" : "false");
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
        string json = "{";
        json += "\"status\":\"success\",";
        json += "\"order_id\":" + to_string(order.id) + ",";
        json += "\"assignedDriverId\":" + to_string(order.assignedDriverId);
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

    // ===== DRIVER =====

    svr.Get("/api/driver/login", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            int driverId = stoi(req.get_param_value("driverId"));
            if (!driverService.driverExists(driverId)) {
                res.set_content("{\"status\":\"error\"}", "application/json");
                return;
            }
            Driver d = driverService.getDriver(driverId);
            string json = "{";
            json += "\"status\":\"success\",";
            json += "\"name\":\"" + jsonEscape(d.name) + "\",";
            json += "\"driverId\":" + to_string(d.driverId);
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
            Driver d = driverService.getDriver(driverId);
            if (d.currentOrderId == -1) {
                res.set_content("{}", "application/json");
                return;
            }
            string json = "{";
            json += "\"id\":"     + to_string(d.currentOrderId) + ",";
            json += "\"items\":\"" + jsonEscape(d.currentItems) + "\",";
            json += "\"address\":\"" + jsonEscape(d.currentAddress) + "\",";
            json += "\"status\":\"IN_DELIVERY\",";
            json += "\"completedDeliveries\":" + to_string(d.completedDeliveries);
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

    // ===== ROUTE =====

    // returns all valid delivery zone names
    svr.Get("/api/zones", [&](const httplib::Request& req, httplib::Response& res) {
        auto zones = graph.getDeliveryZones();
        string json = "[";
        bool first = true;
        for (auto& z : zones) {
            if (!first) json += ",";
            json += "\"" + z + "\"";
            first = false;
        }
        json += "]";
        res.set_content(json, "application/json");
    });

    svr.Get("/api/route", [&](const httplib::Request& req, httplib::Response& res) {
        string dest = req.get_param_value("dest");
        auto result = graph.shortestPath(dest);

        if (result.first == -1) {
            res.set_content(
                "{\"error\":\"Location not in city map\",\"distance\":-1,\"path\":\"\"}",
                "application/json"
            );
            return;
        }

        string json = "{";
        json += "\"distance\":" + to_string(result.first) + ",";
        json += "\"path\":\"" + jsonEscape(result.second) + "\"";
        json += "}";
        res.set_content(json, "application/json");
    });


    // ===== ANALYTICS =====

    svr.Get("/api/analytics/popular", [&](const httplib::Request& req, httplib::Response& res) {
        int limit = 5;
        if (req.has_param("limit")) {
            limit = stoi(req.get_param_value("limit"));
        }

        auto topItems = analytics.getTop(limit);
        string json = "[";
        bool first = true;

        for (auto& item : topItems) {
            if (!first) json += ",";
            json += "{\"item\":\"" + jsonEscape(item.first) + "\",\"count\":" + to_string(item.second) + "}";
            first = false;
        }

        json += "]";
        res.set_content(json, "application/json");
    });

    svr.Get("/api/analytics/stats", [&](const httplib::Request& req, httplib::Response& res) {
        string json = "{";
        json += "\"queue_size\":"          + to_string(orderQueue.size())                + ",";
        json += "\"active_sessions\":"     + to_string(cartManager.getActiveSessionCount()) + ",";
        json += "\"total_drivers\":"       + to_string(driverService.totalDrivers())    + ",";
        json += "\"available_drivers\":"   + to_string(driverService.availableDrivers()) + ",";
        json += "\"history_size\":"        + to_string(orderHistory.size())             + ",";
        json += "\"cart_items\":"          + to_string(cartManager.getTotalCartItems())  + ",";
        json += "\"unique_items_ordered\":" + to_string(analytics.uniqueItemCount())    + ",";
        json += "\"graph_nodes\":"         + to_string(graph.nodeCount())               + ",";
        json += "\"menu_items\":"          + to_string(menuManager.totalItems());
        json += "}";
        res.set_content(json, "application/json");
    });

    cout << "\n========================================\n";
    cout << "  Galle Restaurant - Food Delivery\n";
    cout << "  Web Server: http://localhost:8080/login.html\n";
    cout << "========================================\n";

    svr.listen("localhost", 8080);

    return 0;
}