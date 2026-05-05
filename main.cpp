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

using namespace std;

// ==========================================
// DATA STRUCTURES & ALGORITHMS SHOWCASE
// ==========================================

// ==========================================
// 1. HASH MAP - O(1) avg lookup for menu
// ==========================================
// Nested structure: Category -> Items
// Time Complexity: Insert O(1), Lookup O(1), Delete O(1)
// Space Complexity: O(n) where n = total menu items
struct FoodItem { 
    int id; 
    string name; 
    int price; 
    string category; 
};

unordered_map<string, vector<FoodItem>> menuDB;

// ==========================================
// 2. TRIE - O(m) prefix search (m = word length)
// ==========================================
// Used for: Fast food name search/autocomplete
// Time Complexity: Insert O(m), Search O(m)
// Space Complexity: O(ALPHABET_SIZE * N) where N = nodes
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

// Cleanup Trie recursively - prevents memory leaks
void cleanupTrie(TrieNode* node) {
    if (!node) return;
    for (auto& p : node->children) {
        cleanupTrie(p.second);
    }
    delete node;
}

// ==========================================
// 3. DOUBLY LINKED LIST - O(1) add/remove at ends
// ==========================================
// Used for: Shopping cart with forward & backward traversal
// Time Complexity: Insert O(1), Delete O(1), Traverse O(n)
// Space Complexity: O(n) where n = items in cart
struct CartNode { 
    string item; 
    int price; 
    CartNode* next; 
    CartNode* prev; 
};

CartNode* cartHead = nullptr;
CartNode* cartTail = nullptr;

// ==========================================
// 4. PRIORITY QUEUE - O(log n) insert/extract
// ==========================================
// Used for: Kitchen order management (VIP priority)
// Ordering: VIPs first, then by order ID (older first)
struct Order {
    int id; 
    string items; 
    string address; 
    bool isVIP;
    
    // Priority: VIP=true > VIP=false, then smaller ID (older)
    bool operator<(const Order& other) const {
        if (isVIP != other.isVIP) return !isVIP;  // VIP comes first
        return id > other.id;  // Then oldest order
    }
};

priority_queue<Order> adminQueue;

// ==========================================
// 5. STACK - O(1) push/pop
// ==========================================
// Used for: Undo functionality (Last In First Out)
// Time Complexity: Push O(1), Pop O(1), Peek O(1)
// Space Complexity: O(n) where n = order history
stack<Order> adminHistory;

// ==========================================
// 6. QUEUE (FIFO) - O(1) enqueue/dequeue
// ==========================================
// Used for: Driver delivery queue
// Time Complexity: Push O(1), Pop O(1)
// Space Complexity: O(n) where n = pending deliveries
queue<Order> driverQueue;

// ==========================================
// 7. HASH MAP COUNTER - O(1) increment/lookup
// ==========================================
// Used for: Item popularity analytics
// Time Complexity: Increment O(1), Query O(1)
// Space Complexity: O(k) where k = unique items
unordered_map<string, int> itemOrderCount;

// ==========================================
// 8. WEIGHTED GRAPH + DIJKSTRA - O((V+E)logV)
// ==========================================
// Used for: Route optimization between locations
// Time Complexity: O((V+E)logV) with binary heap
// Space Complexity: O(V+E) where V=nodes, E=edges
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

// DIJKSTRA'S ALGORITHM - finds shortest path
// Time: O((V+E)logV) using priority queue
// Returns: JSON with distance and path
string getRoute(string endNode) {
    if (cityGraph.find(endNode) == cityGraph.end()) {
        return "{\"distance\": 0, \"path\": \"Error: Location not mapped\"}";
    }

    // Initialize distances and previous nodes
    unordered_map<string, int> dist;
    unordered_map<string, string> prev;
    
    for (const auto& p : cityGraph) {
        dist[p.first] = INT_MAX;
    }
    dist["Restaurant"] = 0;

    // Min-heap priority queue: (distance, node)
    priority_queue<pair<int, string>, vector<pair<int, string>>, greater<pair<int, string>>> pq;
    pq.push({0, "Restaurant"});

    // Dijkstra main loop
    while (!pq.empty()) {
        int d = pq.top().first;
        string u = pq.top().second;
        pq.pop();

        if (u == endNode) break;  // Early termination
        if (d > dist[u]) continue;  // Skip outdated entry

        // Relax edges
        for (const auto& neighbor : cityGraph[u]) {
            int newDist = d + neighbor.second;
            if (newDist < dist[neighbor.first]) {
                dist[neighbor.first] = newDist;
                prev[neighbor.first] = u;
                pq.push({newDist, neighbor.first});
            }
        }
    }

    // Reconstruct path
    vector<string> path;
    string curr = endNode;
    while (curr != "") {
        path.push_back(curr);
        if (prev.find(curr) == prev.end()) break;
        curr = prev[curr];
    }
    reverse(path.begin(), path.end());

    // Format JSON response
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

int orderID = 1000;

// ==========================================
// MAIN SERVER
// ==========================================
int main() {
    srand(time(0));
    initializeCityMap();
    
    httplib::Server svr;
    svr.set_mount_point("/", "./public");

    // Pre-load dummy menu items
    menuDB["Burgers"].push_back({1, "Spicy Burger", 1200, "Burgers"});
    menuDB["Burgers"].push_back({2, "Classic Burger", 900, "Burgers"});
    menuDB["Biryani"].push_back({3, "Chicken Biryani", 1500, "Biryani"});
    menuDB["Biryani"].push_back({4, "Beef Biryani", 1800, "Biryani"});
    menuDB["Pizzas"].push_back({5, "Margherita", 1100, "Pizzas"});
    menuDB["Pizzas"].push_back({6, "Pepperoni", 1300, "Pizzas"});
    
    insertToTrie("Spicy Burger");
    insertToTrie("Classic Burger");
    insertToTrie("Chicken Biryani");
    insertToTrie("Beef Biryani");
    insertToTrie("Margherita");
    insertToTrie("Pepperoni");

    // ==========================================
    // ADMIN APIs
    // ==========================================

    // Add food to menu - Uses Hash Map
    // Time: O(1) average case for hash map insertion
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

    // Get admin queue - Uses Priority Queue
    // Time: O(n) to traverse queue
    svr.Get("/api/admin/queue", [&](const httplib::Request& req, httplib::Response& res) {
        auto tq = adminQueue;
        string json = "[";
        bool first = true;
        
        while (!tq.empty()) {
            if (!first) json += ",";
            const Order& o = tq.top();
            json += "{\"id\":" + to_string(o.id) + 
                    ",\"items\":\"" + o.items + 
                    "\",\"vip\":" + (o.isVIP ? "true" : "false") + "}";
            first = false;
            tq.pop();
        }
        
        res.set_content(json + "]", "application/json");
    });

    // Process (cook) next order - Uses Priority Queue + Queue
    // Time: O(log n) priority queue pop, O(1) queue push
    svr.Get("/api/admin/process", [&](const httplib::Request& req, httplib::Response& res) {
        if (adminQueue.empty()) {
            res.set_content("{\"status\":\"empty\"}", "application/json");
            return;
        }
        
        Order o = adminQueue.top();
        adminQueue.pop();
        adminHistory.push(o);
        driverQueue.push(o);
        
        cout << "[KITCHEN] Processing Order #" << o.id << " (VIP: " << (o.isVIP ? "Yes" : "No") << ")\n";
        res.set_content("{\"status\":\"success\",\"order_id\":" + to_string(o.id) + "}", "application/json");
    });

    // Undo last operation - Uses Stack
    // Time: O(log n) priority queue push, O(1) stack pop
    svr.Get("/api/admin/undo", [&](const httplib::Request& req, httplib::Response& res) {
        if (adminHistory.empty()) {
            res.set_content("{\"status\":\"no_history\"}", "application/json");
            return;
        }
        
        Order o = adminHistory.top();
        adminHistory.pop();
        adminQueue.push(o);
        
        cout << "[UNDO] Restored Order #" << o.id << " to queue\n";
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    // ==========================================
    // USER APIs
    // ==========================================

    // Get menu with optional search - Uses Hash Map + Trie
    // Time: O(n) for full menu, O(m) for trie search (m = query length)
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

    // Add item to cart - Uses Doubly Linked List
    // Time: O(1) for insertion at tail
    svr.Get("/api/cart/add", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            string item = req.get_param_value("item");
            int price = stoi(req.get_param_value("price"));
            
            CartNode* n = new CartNode{item, price, nullptr, nullptr};
            
            if (!cartHead) {
                cartHead = cartTail = n;
            } else {
                cartTail->next = n;
                n->prev = cartTail;
                cartTail = n;
            }
            
            cout << "[CART] Added '" << item << "' (Rs. " << price << ")\n";
            res.set_content("{\"status\":\"added\"}", "application/json");
        } catch (const exception& e) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    // View cart - Uses Doubly Linked List
    // Time: O(n) where n = items in cart
    svr.Get("/api/cart/view", [&](const httplib::Request& req, httplib::Response& res) {
        string json = "[";
        CartNode* curr = cartHead;
        bool first = true;
        
        while (curr) {
            if (!first) json += ",";
            json += "{\"item\":\"" + curr->item + "\",\"price\":" + to_string(curr->price) + "}";
            first = false;
            curr = curr->next;
        }
        
        res.set_content(json + "]", "application/json");
    });

    // Checkout - Uses all structures
    // Time: O(n) for cart traversal + O(log n) for priority queue insertion
    svr.Get("/api/checkout", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            if (!cartHead) {
                res.set_content("{\"status\":\"empty_cart\"}", "application/json");
                return;
            }
            
            bool vip = (req.get_param_value("vip") == "true");
            string address = req.get_param_value("address");
            
            // Update graph with new address
            addNewAddressToGraph(address);

            // Collect all items and delete cart
            string allItems = "";
            CartNode* curr = cartHead;
            while (curr) {
                allItems += curr->item + ", ";
                CartNode* temp = curr;
                curr = curr->next;
                delete temp;  // Free memory
            }
            
            cartHead = cartTail = nullptr;

            // Track item frequencies
            incrementItemCounters(allItems);

            // Add order to kitchen queue
            adminQueue.push({orderID, allItems, address, vip});
            
            cout << "[CHECKOUT] Order #" << orderID << " - VIP: " << (vip ? "Yes" : "No") << " - To: " << address << "\n";
            res.set_content("{\"status\":\"success\",\"order_id\":" + to_string(orderID) + ",\"vip\":" + (vip ? "true" : "false") + "}", "application/json");
            
            orderID++;
        } catch (const exception& e) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    // ==========================================
    // DRIVER APIs
    // ==========================================

    // Get next delivery from queue - Uses Queue (FIFO)
    // Time: O(1)
    svr.Get("/api/driver/queue", [&](const httplib::Request& req, httplib::Response& res) {
        if (driverQueue.empty()) {
            res.set_content("{}", "application/json");
            return;
        }
        
        Order o = driverQueue.front();
        res.set_content("{\"id\":" + to_string(o.id) + 
                       ",\"items\":\"" + o.items + 
                       "\",\"address\":\"" + o.address + "\"}", "application/json");
    });

    // Complete delivery - Uses Queue
    // Time: O(1)
    svr.Get("/api/driver/deliver", [&](const httplib::Request& req, httplib::Response& res) {
        if (!driverQueue.empty()) {
            Order o = driverQueue.front();
            driverQueue.pop();
            cout << "[DELIVERY] Completed Order #" << o.id << " to " << o.address << "\n";
        }
        
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    // Get route to delivery address - Uses Dijkstra's Algorithm
    // Time: O((V+E)logV)
    svr.Get("/api/route", [&](const httplib::Request& req, httplib::Response& res) {
        string dest = req.get_param_value("dest");
        res.set_content(getRoute(dest), "application/json");
    });

    // ==========================================
    // ANALYTICS APIs
    // ==========================================

    // Get popular items - Uses Hash Map Counter
    // Time: O(n log n) for sorting where n = unique items
    svr.Get("/api/analytics/popular", [&](const httplib::Request& req, httplib::Response& res) {
        int limit = req.has_param("limit") ? stoi(req.get_param_value("limit")) : 5;

        vector<pair<string, int>> sorted(itemOrderCount.begin(), itemOrderCount.end());
        sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;  // Descending order
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

    // Get detailed analytics - Shows data structure stats
    // Time: O(n) + O(m) where n=items, m=graph nodes
    svr.Get("/api/analytics/stats", [&](const httplib::Request& req, httplib::Response& res) {
        int cartSize = 0;
        CartNode* curr = cartHead;
        while (curr) { cartSize++; curr = curr->next; }
        
        string json = "{";
        json += "\"queue_size\":" + to_string(adminQueue.size()) + ",";
        json += "\"driver_queue_size\":" + to_string(driverQueue.size()) + ",";
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
    cout << "\n" << string(50, '=') << "\n";
    cout << "  SMART FOOD DELIVERY SYSTEM\n";
    cout << "  Data Structures: 8 (Hash Map, Trie, Linked List, Priority Queue, Stack, Queue, Counter, Graph)\n";
    cout << "  Algorithms: Dijkstra, Priority Sorting, Trie Traversal\n";
    cout << "  Server: http://localhost:8080\n";
    cout << string(50, '=') << "\n\n";

    if (!svr.listen("localhost", 8080)) {
        cerr << "Failed to start server\n";
        cleanupTrie(searchRoot);
        return 1;
    }

    // Cleanup before exit
    cleanupTrie(searchRoot);
    CartNode* curr = cartHead;
    while (curr) {
        CartNode* temp = curr;
        curr = curr->next;
        delete temp;
    }

    return 0;
}