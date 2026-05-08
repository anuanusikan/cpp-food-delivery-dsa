#include "Graph.h"
#include <climits>
#include <algorithm>
#include <iostream>

using namespace std;

int Graph::findNodeIndex(string name) {
    for (int i = 0; i < (int)nodes.size(); i++) {
        if (nodes[i].name == name) {
            return i;
        }
    }

    return -1;
}

void Graph::addEdge(string from, string to, int weight) {
    int fromIdx = findNodeIndex(from);

    if (fromIdx == -1) {
        GraphNode n;
        n.name = from;
        nodes.push_back(n);
        fromIdx = nodes.size() - 1;
    }

    // Make sure destination node exists
    if (findNodeIndex(to) == -1) {
        GraphNode n;
        n.name = to;
        nodes.push_back(n);
    }

    Edge e;
    e.destination = to;
    e.weight = weight;

    nodes[fromIdx].edges.push_back(e);
}

/*
    Galle city weighted graph with junctions.

    Users select only main delivery zones:
    - Engineering_Faculty
    - Wakwella
    - Karapitiya_Medical_Faculty
    - Allied_Health_Campus
    - Hikkaduwa
    - Hapugala
    - Galle_Hospital

    The driver sees full path through junctions:

    Galle_Restaurant
        -> Main_Junction
        -> Kalegana_Junction
        -> Kalegana
        -> Hapugala_Junction
        -> Hapugala
        -> Engineering_Faculty
        -> Customer_1000
*/

void Graph::init() {
    nodes.clear();

    // Main roads from restaurant
    addEdge("Galle_Restaurant", "Main_Junction", 1);
    addEdge("Main_Junction", "Galle_Restaurant", 1);

    addEdge("Main_Junction", "Kalegana_Junction", 2);
    addEdge("Kalegana_Junction", "Main_Junction", 2);

    addEdge("Main_Junction", "Kahaduwawaththa_Junction", 3);
    addEdge("Kahaduwawaththa_Junction", "Main_Junction", 3);

    // Kalegana side
    addEdge("Kalegana_Junction", "Kalegana", 1);
    addEdge("Kalegana", "Kalegana_Junction", 1);

    addEdge("Kalegana", "Hapugala_Junction", 2);
    addEdge("Hapugala_Junction", "Kalegana", 2);

    addEdge("Hapugala_Junction", "Hapugala", 1);
    addEdge("Hapugala", "Hapugala_Junction", 1);

    addEdge("Hapugala", "Engineering_Faculty", 1);
    addEdge("Engineering_Faculty", "Hapugala", 1);

    addEdge("Hapugala_Junction", "Wakwella_Road_Junction", 2);
    addEdge("Wakwella_Road_Junction", "Hapugala_Junction", 2);

    addEdge("Wakwella_Road_Junction", "Wakwella", 2);
    addEdge("Wakwella", "Wakwella_Road_Junction", 2);

    // Kahaduwawaththa / Hospital side
    addEdge("Kahaduwawaththa_Junction", "Kahaduwawaththa", 1);
    addEdge("Kahaduwawaththa", "Kahaduwawaththa_Junction", 1);

    addEdge("Kahaduwawaththa", "Hospital_Junction", 2);
    addEdge("Hospital_Junction", "Kahaduwawaththa", 2);

    addEdge("Hospital_Junction", "Galle_Hospital", 1);
    addEdge("Galle_Hospital", "Hospital_Junction", 1);

    addEdge("Galle_Hospital", "Karapitiya_Junction", 2);
    addEdge("Karapitiya_Junction", "Galle_Hospital", 2);

    addEdge("Karapitiya_Junction", "Karapitiya_Medical_Faculty", 1);
    addEdge("Karapitiya_Medical_Faculty", "Karapitiya_Junction", 1);

    addEdge("Galle_Hospital", "Allied_Health_Campus", 1);
    addEdge("Allied_Health_Campus", "Galle_Hospital", 1);

    // Cross roads between the two sides
    addEdge("Kalegana_Junction", "Kahaduwawaththa_Junction", 2);
    addEdge("Kahaduwawaththa_Junction", "Kalegana_Junction", 2);

    addEdge("Hapugala_Junction", "Hospital_Junction", 4);
    addEdge("Hospital_Junction", "Hapugala_Junction", 4);

    addEdge("Wakwella_Road_Junction", "Karapitiya_Junction", 5);
    addEdge("Karapitiya_Junction", "Wakwella_Road_Junction", 5);

    // Hikkaduwa side
    addEdge("Karapitiya_Junction", "Coastal_Road_Junction", 3);
    addEdge("Coastal_Road_Junction", "Karapitiya_Junction", 3);

    addEdge("Coastal_Road_Junction", "Hikkaduwa", 4);
    addEdge("Hikkaduwa", "Coastal_Road_Junction", 4);

    addEdge("Wakwella", "Coastal_Road_Junction", 6);
    addEdge("Coastal_Road_Junction", "Wakwella", 6);

    cout << "[Graph] Galle city map with junctions loaded: " << nodes.size() << " locations\n";
}

void Graph::addAddress(string addr) {
    if (addr.empty()) {
        return;
    }

    if (findNodeIndex(addr) != -1) {
        return;
    }

    // Fallback unknown address connection
    addEdge(addr, "Hapugala", 2);
    addEdge("Hapugala", addr, 2);
}

void Graph::addDynamicUserLocation(string customerNode, string nearestLocation, int distance) {
    if (customerNode.empty() || nearestLocation.empty()) {
        return;
    }

    if (distance <= 0) {
        distance = 1;
    }

    if (findNodeIndex(customerNode) != -1) {
        return;
    }

    if (findNodeIndex(nearestLocation) == -1) {
        nearestLocation = "Galle_Restaurant";
    }

    addEdge(customerNode, nearestLocation, distance);
    addEdge(nearestLocation, customerNode, distance);
}

pair<int, string> Graph::shortestPath(string dest) {
    return shortestPath("Galle_Restaurant", dest);
}

pair<int, string> Graph::shortestPath(string source, string dest) {
    int n = nodes.size();

    int startIdx = findNodeIndex(source);
    int destIdx = findNodeIndex(dest);

    if (startIdx == -1 || destIdx == -1) {
        return {-1, "Location not found in city map"};
    }

    vector<int> dist(n, INT_MAX);
    vector<bool> visited(n, false);
    vector<int> previous(n, -1);

    dist[startIdx] = 0;

    for (int count = 0; count < n - 1; count++) {
        int minDist = INT_MAX;
        int u = -1;

        for (int i = 0; i < n; i++) {
            if (!visited[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }

        if (u == -1) {
            break;
        }

        visited[u] = true;

        for (Edge& edge : nodes[u].edges) {
            int v = findNodeIndex(edge.destination);

            if (v != -1 && !visited[v] && dist[u] != INT_MAX) {
                int newDist = dist[u] + edge.weight;

                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    previous[v] = u;
                }
            }
        }
    }

    if (dist[destIdx] == INT_MAX) {
        return {-1, "No path found"};
    }

    vector<string> path;
    int current = destIdx;

    while (current != -1) {
        path.push_back(nodes[current].name);
        current = previous[current];
    }

    reverse(path.begin(), path.end());

    string pathText = "";

    for (int i = 0; i < (int)path.size(); i++) {
        pathText += path[i];

        if (i < (int)path.size() - 1) {
            pathText += " -> ";
        }
    }

    return {dist[destIdx], pathText};
}

int Graph::distanceBetween(string source, string dest) {
    pair<int, string> result = shortestPath(source, dest);

    if (result.first < 0) {
        return INT_MAX / 2;
    }

    return result.first;
}

pair<int, string> Graph::optimizedMultiStopRoute(vector<string> deliveryLocations) {
    if (deliveryLocations.empty()) {
        return {0, "No deliveries"};
    }

    vector<bool> visited(deliveryLocations.size(), false);

    string currentLocation = "Galle_Restaurant";
    string fullRoute = "Galle_Restaurant";
    int totalDistance = 0;
    int deliveredCount = 0;

    while (deliveredCount < (int)deliveryLocations.size()) {
        int bestIndex = -1;
        int bestDistance = INT_MAX;

        for (int i = 0; i < (int)deliveryLocations.size(); i++) {
            if (!visited[i]) {
                int d = distanceBetween(currentLocation, deliveryLocations[i]);

                if (d < bestDistance) {
                    bestDistance = d;
                    bestIndex = i;
                }
            }
        }

        if (bestIndex == -1) {
            break;
        }

        pair<int, string> routePart = shortestPath(currentLocation, deliveryLocations[bestIndex]);

        if (routePart.first < 0) {
            break;
        }

        totalDistance += routePart.first;

        string partPath = routePart.second;

        size_t arrowPosition = partPath.find(" -> ");

        if (arrowPosition != string::npos) {
            partPath = partPath.substr(arrowPosition + 4);
        }

        fullRoute += " -> " + partPath;

        currentLocation = deliveryLocations[bestIndex];
        visited[bestIndex] = true;
        deliveredCount++;
    }

    return {totalDistance, fullRoute};
}

vector<string> Graph::getDeliveryZones() {
    vector<string> zones;

    zones.push_back("Engineering_Faculty");
    zones.push_back("Wakwella");
    zones.push_back("Karapitiya_Medical_Faculty");
    zones.push_back("Allied_Health_Campus");
    zones.push_back("Hikkaduwa");
    zones.push_back("Hapugala");
    zones.push_back("Galle_Hospital");

    return zones;
}

vector<string> Graph::getPredefinedLocations() {
    return getDeliveryZones();
}

int Graph::nodeCount() {
    return (int)nodes.size();
}