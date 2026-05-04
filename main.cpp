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

using namespace std;

// ==========================================
// DATA STRUCTURES
// ==========================================

// 1. Structs
struct FoodItem { int id; string name; int price; string category; };

struct Order {
    int id; string items; string address; bool isVIP;
    // Priority Queue Sorting Logic (VIPs first, then older orders)
    bool operator<(const Order& other) const {
        if (isVIP != other.isVIP) return !isVIP;
        return id > other.id;
    }
};

// 2. Nested Hash Map (Menu)
unordered_map<string, vector<FoodItem>> menuDB;

// 3. Trie (Fast Search)
struct TrieNode { unordered_map<char, TrieNode*> children; bool isEndOfWord = false; };
TrieNode* searchRoot = new TrieNode();

void insertToTrie(string word) {
    TrieNode* curr = searchRoot;
    for (char c : word) {
        c = tolower(c);
        if (!curr->children[c]) curr->children[c] = new TrieNode();
        curr = curr->children[c];
    }
    curr->isEndOfWord = true;
}

// 4. Doubly Linked List (Cart)
struct CartNode { string item; int price; CartNode* next; CartNode* prev; };
CartNode* cartHead = nullptr; CartNode* cartTail = nullptr;

// 5. Priority Queue & Stack (Admin)
priority_queue<Order> adminQueue;
stack<Order> adminHistory;
int orderID = 1000;

// 6. Queue (Driver)
queue<Order> driverQueue;

// 7. Item Frequency Counter (Analytics)
unordered_map<string, int> itemOrderCount;

// 8. Dynamic Graph (Routing)
unordered_map<string, vector<pair<string, int>>> cityGraph;

void initializeCityMap() {
    cityGraph["Restaurant"] = {{"Junction_A", 4}, {"Junction_B", 2}};
    cityGraph["Junction_A"] = {{"Restaurant", 4}, {"Main_Road", 5}};
    cityGraph["Junction_B"] = {{"Restaurant", 2}, {"Junction_A", 1}, {"Main_Road", 8}};
    cityGraph["Main_Road"] = {{"Junction_A", 5}, {"Junction_B", 8}};
}

void addNewAddressToGraph(string newAddress) {
    if (cityGraph.find(newAddress) != cityGraph.end()) return; // Already exists
    int randomDistance = (rand() % 5) + 1; // 1 to 5 km from Main Road
    cityGraph[newAddress].push_back({"Main_Road", randomDistance});
    cityGraph["Main_Road"].push_back({newAddress, randomDistance});
    cout << "[GRAPH] Added Node: " << newAddress << " (Distance to Main Road: " << randomDistance << "km)\n";
}

void incrementItemCounters(const string& allItems) {
    stringstream ss(allItems);
    string item;
    while(getline(ss, item, ',')) {
        item.erase(0, item.find_first_not_of(" "));
        item.erase(item.find_last_not_of(" ") + 1);
        if(!item.empty()) {
            itemOrderCount[item]++;
            cout << "[COUNTER] " << item << " count: " << itemOrderCount[item] << endl;
        }
    }
}

string getRoute(string endNode) {
    if(cityGraph.find(endNode) == cityGraph.end()) return "{\"distance\": 0, \"path\": \"Error: Location not mapped\"}";

    unordered_map<string, int> dist; unordered_map<string, string> prev;
    for (const auto& p : cityGraph) dist[p.first] = INT_MAX;
    dist["Restaurant"] = 0;

    priority_queue<pair<int, string>, vector<pair<int, string>>, greater<pair<int, string>>> pq;
    pq.push({0, "Restaurant"});

    while (!pq.empty()) {
        int d = pq.top().first; string u = pq.top().second; pq.pop();
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
    vector<string> path; string curr = endNode;
    while (curr != "") { path.push_back(curr); if(prev.find(curr)==prev.end()) break; curr = prev[curr]; }
    reverse(path.begin(), path.end());

    string json = "{\"distance\": " + to_string(dist[endNode]) + ", \"path\": \"";
    for (size_t i=0; i<path.size(); ++i) json += path[i] + (i<path.size()-1 ? " -> " : "");
    return json + "\"}";
}

// ==========================================
// MAIN SERVER
// ==========================================
int main() {
    srand(time(0));
    initializeCityMap();
    httplib::Server svr;
    svr.set_mount_point("/", "./public");

    // Pre-load dummy data
    menuDB["Burgers"].push_back({1, "Spicy Burger", 1200, "Burgers"});
    insertToTrie("Spicy Burger");

    // --- ADMIN APIs ---
    svr.Get("/api/menu/add", [&](const httplib::Request& req, httplib::Response& res) {
        int id = stoi(req.get_param_value("id"));
        string name = req.get_param_value("name");
        int price = stoi(req.get_param_value("price"));
        string cat = req.get_param_value("category");
        menuDB[cat].push_back({id, name, price, cat});
        insertToTrie(name);
        cout << "[DB] Added " << name << " to " << cat << endl;
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    svr.Get("/api/admin/queue", [&](const httplib::Request& req, httplib::Response& res) {
        auto tq = adminQueue; string json = "["; bool first = true;
        while(!tq.empty()) {
            if(!first) json += ",";
            json += "{\"id\":" + to_string(tq.top().id) + ",\"items\":\"" + tq.top().items + "\",\"vip\":" + (tq.top().isVIP?"true":"false") + "}";
            first = false; tq.pop();
        }
        res.set_content(json + "]", "application/json");
    });

    svr.Get("/api/admin/process", [&](const httplib::Request& req, httplib::Response& res) {
        if(adminQueue.empty()) return;
        Order o = adminQueue.top(); adminQueue.pop();
        adminHistory.push(o); driverQueue.push(o); 
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    svr.Get("/api/admin/undo", [&](const httplib::Request& req, httplib::Response& res) {
        if(adminHistory.empty()) return;
        Order o = adminHistory.top(); adminHistory.pop();
        adminQueue.push(o);
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    // --- USER APIs ---
    svr.Get("/api/menu", [&](const httplib::Request& req, httplib::Response& res) {
        string q = req.has_param("q") ? req.get_param_value("q") : "";
        string json = "{"; bool firstCat = true;
        for (const auto& categoryPair : menuDB) {
            string itemsJson = "["; bool firstItem = true;
            for(const auto& item : categoryPair.second) {
                string nameLower = item.name; transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                string qLower = q; transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);
                if (q == "" || nameLower.find(qLower) == 0) {
                    if(!firstItem) itemsJson += ",";
                    itemsJson += "{\"id\":" + to_string(item.id) + ",\"name\":\"" + item.name + "\",\"price\":" + to_string(item.price) + "}";
                    firstItem = false;
                }
            }
            itemsJson += "]";
            if(itemsJson != "[]") {
                if(!firstCat) json += ",";
                json += "\"" + categoryPair.first + "\":" + itemsJson;
                firstCat = false;
            }
        }
        res.set_content(json + "}", "application/json");
    });

    svr.Get("/api/cart/add", [&](const httplib::Request& req, httplib::Response& res) {
        string item = req.get_param_value("item"); int price = stoi(req.get_param_value("price"));
        CartNode* n = new CartNode{item, price, nullptr, nullptr};
        if(!cartHead) cartHead = cartTail = n; 
        else { cartTail->next = n; n->prev = cartTail; cartTail = n; }
        res.set_content("{\"status\":\"added\"}", "application/json");
    });

    svr.Get("/api/cart/view", [&](const httplib::Request& req, httplib::Response& res) {
        string json = "["; CartNode* curr = cartHead; bool first = true;
        while(curr) {
            if(!first) json += ",";
            json += "{\"item\":\"" + curr->item + "\",\"price\":" + to_string(curr->price) + "}";
            first = false; curr = curr->next;
        }
        res.set_content(json + "]", "application/json");
    });

    svr.Get("/api/checkout", [&](const httplib::Request& req, httplib::Response& res) {
        if(!cartHead) return;
        bool vip = (req.get_param_value("vip") == "true");
        string address = req.get_param_value("address");
        
        addNewAddressToGraph(address); // Update dynamic graph

        string allItems = ""; CartNode* curr = cartHead;
        while(curr) { allItems += curr->item + ", "; CartNode* temp=curr; curr=curr->next; delete temp; }

        incrementItemCounters(allItems);  // Track item popularity
        cout << "[CHECKOUT] Updated item frequencies. Total unique items tracked: " << itemOrderCount.size() << endl;

        cartHead = cartTail = nullptr;

        adminQueue.push({orderID++, allItems, address, vip}); // Pass address to order
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    // --- DRIVER APIs ---
    svr.Get("/api/driver/queue", [&](const httplib::Request& req, httplib::Response& res) {
        if(driverQueue.empty()) { res.set_content("{}", "application/json"); return; }
        Order o = driverQueue.front();
        res.set_content("{\"id\":" + to_string(o.id) + ",\"items\":\"" + o.items + "\",\"address\":\"" + o.address + "\"}", "application/json");
    });

    svr.Get("/api/driver/deliver", [&](const httplib::Request& req, httplib::Response& res) {
        if(!driverQueue.empty()) driverQueue.pop();
        res.set_content("{\"status\":\"success\"}", "application/json");
    });

    svr.Get("/api/route", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_content(getRoute(req.get_param_value("dest")), "application/json");
    });

    // --- ANALYTICS APIs ---
    svr.Get("/api/analytics/popular", [&](const httplib::Request& req, httplib::Response& res) {
        int limit = req.has_param("limit") ? stoi(req.get_param_value("limit")) : 5;

        vector<pair<string, int>> sorted(itemOrderCount.begin(), itemOrderCount.end());
        sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });

        string json = "[";
        bool first = true;
        for(int i = 0; i < min(limit, (int)sorted.size()); i++) {
            if(!first) json += ",";
            json += "{\"item\":\"" + sorted[i].first + "\",\"count\":" + to_string(sorted[i].second) + "}";
            first = false;
        }
        json += "]";

        res.set_content(json, "application/json");
    });

    cout << "\n=======================================\n";
    cout << "MASTER SYSTEM ONLINE\n";
    cout << "Open Browser at: http://localhost:8080\n";
    cout << "=======================================\n";
    svr.listen("localhost", 8080);
    return 0;
}