#pragma once
#include <string>
#include <vector>

using namespace std;

struct Edge {
    string destination;
    int weight;
};

struct GraphNode {
    string name;
    vector<Edge> edges;
};

class Graph {
private:
    vector<GraphNode> nodes;

    int findNodeIndex(string name);
    void addEdge(string from, string to, int weight);

public:
    void init();

    // Keeps compatibility with older code
    void addAddress(string addr);

    // New dynamic customer location support
    void addDynamicUserLocation(string customerNode, string nearestLocation, int distance);

    // Dijkstra from restaurant to destination
    pair<int, string> shortestPath(string dest);

    // Dijkstra from any source to any destination
    pair<int, string> shortestPath(string source, string dest);

    // Distance only
    int distanceBetween(string source, string dest);

    // Multi-order route path for driver dashboard
    pair<int, string> optimizedMultiStopRoute(vector<string> deliveryLocations);

    // Delivery zone list for frontend/user selection
    vector<string> getDeliveryZones();

    // Compatibility name if any code uses getPredefinedLocations()
    vector<string> getPredefinedLocations();

    int nodeCount();
};