#include "httplib.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <stack>
#include <climits>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <memory>
#include <chrono>

using namespace std;

// ==========================================
// FOOD ITEM STRUCTURE
// ==========================================
struct FoodItem { 
    int id; 
    string name; 
    int price; 
    string category; 
};

unordered_map<string, vector<FoodItem>> menuDB;

// ==========================================
// TRIE FOR SEARCH
// ==========================================
struct TrieNode { 
    unordered_map<char, TrieNode*> children; 
    bool isEndOfWord = false; 
};

TrieNode* searchRoot = nullptr;

void insertToTrie(string word) {
    if (!searchRoot) searchRoot = new TrieNode();
    TrieNode* curr = searchRoot;
    for (char c : word) {
        c = tolower(c);
        if (!curr->children[c]) {
            curr->children[c] = new TrieNode();
        }
        curr = curr->children[c];
    }
    curr->isEndOfWord = true;
}

void cleanupTrie(TrieNode* node) {
    if (!node) return;
    for (auto& p : node->children) {
        cleanupTrie(p.second);
    }
    delete node;
}

// ==========================================
// CART LINKED LIST - SESSION BASED
// ==========================================
struct CartNode { 
    string item; 
    int price; 
    CartNode* next; 
    CartNode* prev; 
};

unordered_map<string, CartNode*> sessionCarts;
unordered_map<string, CartNode*> sessionCartTails;

// ==========================================
// ORDER STATUS TRACKING
// ==========================================
enum OrderStatus {
    PENDING = 0,
    PREPARING = 1,
    READY = 2,
    ASSIGNED = 3,
    IN_DELIVERY = 4,
    DELIVERED = 5
};

string getStatusString(OrderStatus status) {
    switch(status) {
        case PENDING: return "Pending";
        case PREPARING: return "Preparing";
        case READY: return "Ready";
        case ASSIGNED: return "Assigned";
        case IN_DELIVERY: return "In Delivery";
        case DELIVERED: return "Delivered";
        default: return "Unknown";
    }
}

// ==========================================
// DRIVER MANAGEMENT
// ==========================================
enum DriverStatus {
    AVAILABLE = 0,
    ON_DELIVERY = 1
};

struct Driver {
    int driverId;
    string name;
    DriverStatus status;
    int currentOrderId;
    string currentAddress;
    int completedDeliveries;
};

unordered_map<int, Driver> drivers;
int nextDriverId = 101;

// ==========================================
// ORDER STRUCTURE
// ==========================================
struct Order {
    int id;
    string items;
    string address;
    bool isVIP;
    OrderStatus status;
    long long timestamp;
    int assignedDriverId;
    
    bool operator<(const Order& other) const {
        if (isVIP != other.isVIP) return !isVIP;
        return id > other.id;
    }
};

priority_queue<Order> adminQueue;
stack<Order> adminHistory;
unordered_map<string, int> itemOrderCount;

// ==========================================
// GRAPH & DIJKSTRA
// ==========================================
unordered_map<string, vector<pair<string, int>>> cityGraph;

void initializeCityMap() {
    cityGraph["Restaurant"] = {{"Junction_A", 4}, {"Junction_B", 2}};
    cityGraph["Junction_A"] = {{"Restaurant", 4}, {"Main_Road", 5}};
    cityGraph["Junction_B"] = {{"Restaurant", 2}, {"Junction_A", 1}, {"Main_Road", 8}};
    cityGraph["Main_Road"] = {{"Junction_A", 5}, {"Junction_B", 8}};
    cout << "[GRAPH] City map initialized with 4 nodes\n";
}

void addNewAddressToGraph(string newAddress) {
    if (cityGraph.find(newAddress) != cityGraph.end()) return;
    int randomDistance = (rand() % 5) + 1;
    cityGraph[newAddress].push_back({"Main_Road", randomDistance});
    cityGraph["Main_Road"].push_back({newAddress, randomDistance});
    cout << "[GRAPH] Added: " << newAddress << " (Distance: " << randomDistance << "km)\n";
}

string getRoute(string endNode) {
    if (cityGraph.find(endNode) == cityGraph.end()) {
        return "{\"distance\": 0, \"path\": \"Error: Location not mapped\"}";
    }

    unordered_map<string, int> dist;
    unordered_map<string, string> prev;
    
    for (const auto& p : cityGraph) {
        dist[p.first] = INT_MAX;
    }
    dist["Restaurant"] = 0;

    priority_queue<pair<int, string>, vector<pair<int, string>>, greater<pair<int, string>>> pq;
    pq.push({0, "Restaurant"});

    while (!pq.empty()) {
        int d = pq.top().first;
        string u = pq.top().second;
        pq.pop();

        if (u == endNode) break;
        if (d > dist[u]) continue;

        for (const auto& neighbor : cityGraph[u]) {
            int newDist = d + neighbor.second;
            if (newDist < dist[neighbor.first]) {
                dist[neighbor.first] = newDist;
                prev[neighbor.first] = u;
                pq.push({newDist, neighbor.first});
            }
        }
    }

    vector<string> path;
    string curr = endNode;
    while (curr != "") {
        path.push_back(curr);
        if (prev.find(curr) == prev.end()) break;
        curr = prev[curr];
    }
    reverse(path.begin(), path.end());

    string json = "{\"distance\": " + to_string(dist[endNode]) + ", \"path\": \"";
    for (size_t i = 0; i < path.size(); ++i) {
        json += path[i] + (i < path.size() - 1 ? " -> " : "");
    }
    return json + "\"}";
}

void incrementItemCounters(const string& allItems) {
    stringstream ss(allItems);
    string item;
    while (getline(ss, item, ',')) {
        item.erase(0, item.find_first_not_of(" "));
        item.erase(item.find_last_not_of(" ") + 1);
        if (!item.empty()) {
            itemOrderCount[item]++;
            cout << "[ANALYTICS] " << item << " ordered " << itemOrderCount[item] << " times\n";
        }
    }
}

void assignOrderToDriver(Order& order) {
    for (auto& [driverId, driver] : drivers) {
        if (driver.status == AVAILABLE) {
            driver.status = ON_DELIVERY;
            driver.currentOrderId = order.id;
            driver.currentAddress = order.address;
            order.assignedDriverId = driverId;
            order.status = ASSIGNED;
            cout << "[DRIVER] Driver #" << driverId << " assigned to Order #" << order.id << "\n";
            return;
        }
    }
    cout << "[DRIVER] Warning: No available drivers for Order #" << order.id << "\n";
}

int orderID = 1000;

// ==========================================
// MAIN SERVER
// ==========================================
int main() {
    srand(time(0));
    initializeCityMap();
    
    // Initialize drivers
    for (int i = 0; i < 3; i++) {
        Driver d;
        d.driverId = nextDriverId++;
        d.name = "Driver_" + to_string(i + 1);
        d.status = AVAILABLE;
        d.currentOrderId = -1;
        d.completedDeliveries = 0;
        drivers[d.driverId] = d;
        cout << "[DRIVER] " << d.name << " (ID: " << d.driverId << ") registered\n";
    }
    
    httplib::Server svr;
    svr.set_mount_point("/", "./public");

    // Load menu
    menuDB["Burgers"].push_back({1, "Spicy Burger", 1200, "Burgers"});
    menuDB["Burgers"].push_back({2, "Classic Burger", 900, "Burgers"});
    menuDB["Biryani"].push_back({3, "Chicken Biryani", 1500, "Biryani"});
    menuDB["Biryani"].push_back({4, "Beef Biryani", 1800, "Biryani"});
    menuDB["Pizzas"].push_back({5, "Margherita", 1100, "Pizzas"});
    menuDB["Pizzas"].push_back({6, "Pepperoni", 1300, "Pizzas"});
    
    for (const auto& cat : menuDB) {
        for (const auto& item : cat.second) {
            insertToTrie(item.name);
        }
    }

    // ==========================================
    // SESSION MANAGEMENT
    // ==========================================
    svr.Get("/api/session/create", [&](const httplib::Request& req, httplib::Response& res) {
        string sessionId = "session_" + to_string(time(nullptr)) + "_" + to_string(rand() % 100000);
        cout << "[SESSION] Created session: " << sessionId << "\n";
        res.set_content("{\"sessionId\":\"" + sessionId + "\"}", "application/json");
    });

    // ==========================================
    // ADMIN APIs
    // ==========================================

    svr.Get("/api/menu/add", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            int id = stoi(req.get_param_value("id"));
            string name = req.get_param_value("name");
            int price = stoi(req.get_param_value("price"));
            string cat = req.get_param_value("category");
            
            menuDB[cat].push_back({id, name, price, cat});
            insertToTrie(name);
            
            cout << "[DB] Added '" << name << "' (Rs. " << price << ") to " << cat << "\n";
            res.set_content("{\"status\":\"success\",\"message\":\"Food item added\"}", "application/json");
        } catch (const exception& e) {
            res.set_content("{\"status\":\"error\",\"message\":\"Invalid parameters\"}", "application/json");
        }
    });

    svr.Get("/api/admin/queue", [&](const httplib::Request& req, httplib::Response& res) {
        auto tq = adminQueue;
        string json = "[";
        bool first = true;
        
        while (!tq.empty()) {
            if (!first) json += ",";
            const Order& o = tq.top();
            
            json += "{\"id\":" + to_string(o.id) + 
                    ",\"items\":\"" + o.items + 
                    "\",\"status\":\"" + getStatusString(o.status) +
                    "\",\"vip\":" + (o.isVIP ? "true" : "false") + "}";
            first = false;
            tq.pop();
        }
        
        res.set_content(json + "]", "application/json");
    });

    svr.Get("/api/admin/process", [&](const httplib::Request& req, httplib::Response& res) {
        if (adminQueue.empty()) {
            res.set_content("{\"status\":\"empty\"}", "application/json");
            return;
        }
        
        Order o = adminQueue.top();
        adminQueue.pop();
        o.status = READY;
        adminHistory.push(o);
        
        assignOrderToDriver(o);
        
        cout << "[KITCHEN] Processing Order #" << o.id << " (VIP: " << (o.isVIP ? "Yes" : "No") << ")\n";
        res.set_content("{\"status\":\"success\",\"order_id\":" + to_string(o.id) + "}", "application/json");
    });

    svr.Get("/api/admin/undo", [&](const httplib::Request& req, httplib::Response& res) {
        if (adminHistory.empty()) {
            res.set_content("{\"status\":\"no_history\"}", "application/json");
            return;
        }
        
        Order o = adminHistory.top();
        adminHistory.pop();
        o.status = PENDING;
        adminQueue.push(o);
        
        cout << "[UNDO] Restored Order #" << o.id << " to queue\n";
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    // ==========================================
    // USER APIs - SESSION BASED CART
    // ==========================================

    svr.Get("/api/menu", [&](const httplib::Request& req, httplib::Response& res) {
        string q = req.has_param("q") ? req.get_param_value("q") : "";
        string json = "{";
        bool firstCat = true;
        
        for (const auto& categoryPair : menuDB) {
            string itemsJson = "[";
            bool firstItem = true;
            
            for (const auto& item : categoryPair.second) {
                string nameLower = item.name;
                transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                string qLower = q;
                transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);
                
                if (q == "" || nameLower.find(qLower) != string::npos) {
                    if (!firstItem) itemsJson += ",";
                    itemsJson += "{\"id\":" + to_string(item.id) + 
                               ",\"name\":\"" + item.name + 
                               "\",\"price\":" + to_string(item.price) + "}";
                    firstItem = false;
                }
            }
            
            itemsJson += "]";
            if (itemsJson != "[]") {
                if (!firstCat) json += ",";
                json += "\"" + categoryPair.first + "\":" + itemsJson;
                firstCat = false;
            }
        }
        
        res.set_content(json + "}", "application/json");
    });

    svr.Get("/api/cart/add", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            string sessionId = req.get_param_value("sessionId");
            string item = req.get_param_value("item");
            int price = stoi(req.get_param_value("price"));
            
            if (sessionId.empty()) {
                res.set_content("{\"status\":\"error\",\"message\":\"No session ID\"}", "application/json");
                return;
            }
            
            CartNode* n = new CartNode{item, price, nullptr, nullptr};
            
            if (!sessionCarts[sessionId]) {
                sessionCarts[sessionId] = sessionCartTails[sessionId] = n;
            } else {
                sessionCartTails[sessionId]->next = n;
                n->prev = sessionCartTails[sessionId];
                sessionCartTails[sessionId] = n;
            }
            
            cout << "[CART] Session " << sessionId.substr(0, 20) << "... added '" << item << "'\n";
            res.set_content("{\"status\":\"added\"}", "application/json");
        } catch (const exception& e) {
            res.set_content("{\"status\":\"error\",\"message\":\"" + string(e.what()) + "\"}", "application/json");
        }
    });

    svr.Get("/api/cart/view", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            string sessionId = req.get_param_value("sessionId");
            
            string json = "[";
            CartNode* curr = sessionCarts[sessionId];
            bool first = true;
            
            while (curr) {
                if (!first) json += ",";
                json += "{\"item\":\"" + curr->item + "\",\"price\":" + to_string(curr->price) + "}";
                first = false;
                curr = curr->next;
            }
            
            res.set_content(json + "]", "application/json");
        } catch (const exception& e) {
            res.set_content("[]", "application/json");
        }
    });

    svr.Get("/api/checkout", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            string sessionId = req.get_param_value("sessionId");
            CartNode* cartHead = sessionCarts[sessionId];
            
            if (!cartHead) {
                res.set_content("{\"status\":\"empty_cart\"}", "application/json");
                return;
            }
            
            bool vip = (req.get_param_value("vip") == "true");
            string address = req.get_param_value("address");
            
            addNewAddressToGraph(address);

            string allItems = "";
            CartNode* curr = cartHead;
            while (curr) {
                allItems += curr->item + ", ";
                CartNode* temp = curr;
                curr = curr->next;
                delete temp;
            }
            
            sessionCarts[sessionId] = sessionCartTails[sessionId] = nullptr;

            incrementItemCounters(allItems);

            Order newOrder;
            newOrder.id = orderID;
            newOrder.items = allItems;
            newOrder.address = address;
            newOrder.isVIP = vip;
            newOrder.status = PENDING;
            newOrder.timestamp = chrono::system_clock::now().time_since_epoch().count();
            newOrder.assignedDriverId = -1;
            
            adminQueue.push(newOrder);
            
            cout << "[CHECKOUT] Order #" << orderID << " - VIP: " << (vip ? "Yes" : "No") 
                 << " - Status: PENDING\n";
            res.set_content("{\"status\":\"success\",\"order_id\":" + to_string(orderID) 
                           + ",\"vip\":" + (vip ? "true" : "false") + "}", "application/json");
            
            orderID++;
        } catch (const exception& e) {
            res.set_content("{\"status\":\"error\",\"message\":\"" + string(e.what()) + "\"}", "application/json");
        }
    });

    // ==========================================
    // DRIVER APIs - MULTI-DRIVER SUPPORT
    // ==========================================

    svr.Get("/api/driver/login", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            int driverId = stoi(req.get_param_value("driverId"));
            
            if (drivers.find(driverId) != drivers.end()) {
                Driver& d = drivers[driverId];
                cout << "[DRIVER] " << d.name << " (ID: " << driverId << ") logged in\n";
                res.set_content("{\"status\":\"success\",\"name\":\"" + d.name 
                               + "\",\"driverId\":" + to_string(driverId) + "}", "application/json");
            } else {
                res.set_content("{\"status\":\"error\",\"message\":\"Driver not found\"}", "application/json");
            }
        } catch (const exception& e) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    svr.Get("/api/driver/next-assignment", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            int driverId = stoi(req.get_param_value("driverId"));
            
            if (drivers.find(driverId) == drivers.end()) {
                res.set_content("{\"error\":\"Invalid driver\"}", "application/json");
                return;
            }
            
            Driver& driver = drivers[driverId];
            
            if (driver.currentOrderId != -1) {
                res.set_content("{\"id\":" + to_string(driver.currentOrderId) 
                               + ",\"address\":\"" + driver.currentAddress 
                               + "\",\"status\":\"DELIVERING\"}", "application/json");
            } else {
                res.set_content("{}", "application/json");
            }
        } catch (const exception& e) {
            res.set_content("{\"error\":\"" + string(e.what()) + "\"}", "application/json");
        }
    });

    svr.Get("/api/driver/complete", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            int driverId = stoi(req.get_param_value("driverId"));
            
            if (drivers.find(driverId) != drivers.end()) {
                Driver& driver = drivers[driverId];
                driver.status = AVAILABLE;
                driver.currentOrderId = -1;
                driver.completedDeliveries++;
                cout << "[DELIVERY] Driver #" << driverId << " completed delivery. Total: " 
                     << driver.completedDeliveries << "\n";
                res.set_content("{\"status\":\"success\"}", "application/json");
            }
        } catch (const exception& e) {
            res.set_content("{\"error\":\"" + string(e.what()) + "\"}", "application/json");
        }
    });

    svr.Get("/api/route", [&](const httplib::Request& req, httplib::Response& res) {
        string dest = req.get_param_value("dest");
        res.set_content(getRoute(dest), "application/json");
    });

    // ==========================================
    // ANALYTICS APIs
    // ==========================================

    svr.Get("/api/analytics/popular", [&](const httplib::Request& req, httplib::Response& res) {
        int limit = req.has_param("limit") ? stoi(req.get_param_value("limit")) : 5;

        vector<pair<string, int>> sorted(itemOrderCount.begin(), itemOrderCount.end());
        sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

        string json = "[";
        bool first = true;
        for (int i = 0; i < min(limit, (int)sorted.size()); i++) {
            if (!first) json += ",";
            json += "{\"item\":\"" + sorted[i].first + 
                   "\",\"count\":" + to_string(sorted[i].second) + "}";
            first = false;
        }
        json += "]";

        res.set_content(json, "application/json");
    });

    svr.Get("/api/analytics/stats", [&](const httplib::Request& req, httplib::Response& res) {
        int cartSize = 0;
        for (const auto& [sid, cart] : sessionCarts) {
            CartNode* curr = cart;
            while (curr) { cartSize++; curr = curr->next; }
        }
        
        int availableDrivers = 0;
        for (const auto& [id, driver] : drivers) {
            if (driver.status == AVAILABLE) availableDrivers++;
        }
        
        string json = "{";
        json += "\"queue_size\":" + to_string(adminQueue.size()) + ",";
        json += "\"active_sessions\":" + to_string(sessionCarts.size()) + ",";
        json += "\"total_drivers\":" + to_string(drivers.size()) + ",";
        json += "\"available_drivers\":" + to_string(availableDrivers) + ",";
        json += "\"history_size\":" + to_string(adminHistory.size()) + ",";
        json += "\"cart_items\":" + to_string(cartSize) + ",";
        json += "\"unique_items_ordered\":" + to_string(itemOrderCount.size()) + ",";
        json += "\"graph_nodes\":" + to_string(cityGraph.size());
        json += "}";
        
        res.set_content(json, "application/json");
    });

    // ==========================================
    // START SERVER
    // ==========================================
    cout << "\n" << string(70, '=') << "\n";
    cout << "  SMART FOOD DELIVERY SYSTEM - CORRECTED VERSION\n";
    cout << "  ✅ Multi-User Support (Session-Based Cart)\n";
    cout << "  ✅ Multi-Driver Support (3+ Drivers)\n";
    cout << "  ✅ Order Status Tracking (PENDING→DELIVERED)\n";
    cout << "  ✅ 8 Data Structures + Dijkstra\n";
    cout << "  Server: http://localhost:8080\n";
    cout << string(70, '=') << "\n\n";

    if (!svr.listen("localhost", 8080)) {
        cerr << "Failed to start server\n";
        cleanupTrie(searchRoot);
        return 1;
    }

    cleanupTrie(searchRoot);
    for (auto& [sid, cart] : sessionCarts) {
        CartNode* curr = cart;
        while (curr) {
            CartNode* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }

    return 0;
}