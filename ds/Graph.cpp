#include "Graph.h"
#include <climits>
#include <algorithm>
#include <iostream>

using namespace std;

int Graph::findNodeIndex(string name) {
    for (int i = 0; i < (int)nodes.size(); i++) {
        if (nodes[i].name == name) return i;
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

    Edge e;
    e.destination = to;
    e.weight = weight;
    nodes[fromIdx].edges.push_back(e);
}

/*
  City Map Layout - Galle, Sri Lanka (fixed distances):

              Galle_Restaurant
                /           \
           (3km)             (4km)
              /                 \
         Kalegana ----(1km)---- Kahaduwawaththa
              |                       |
           (2km)                   (3km)
              |                       |
          Hapugala             Galle_Hospital
           /    \                  /        \
       (1km)   (2km)           (2km)        (1km)
         /        \              /               \
  Engineering_Faculty  Wakwella  Karapitiya_Medical_Faculty  Allied_Health_Campus
                                       |
                                    (4km)
                                   Hikkaduwa
*/

void Graph::init() {
    // Galle Restaurant to main road intersections
    addEdge("Galle_Restaurant",  "Kalegana",         3);
    addEdge("Kalegana",          "Galle_Restaurant", 3);
    addEdge("Galle_Restaurant",  "Kahaduwawaththa",  4);
    addEdge("Kahaduwawaththa",   "Galle_Restaurant", 4);

    // Between intersections (Kalegana Flyover to Kahaduwawaththa)
    addEdge("Kalegana",         "Kahaduwawaththa",  1);
    addEdge("Kahaduwawaththa",  "Kalegana",         1);

    // Intersections to area hubs
    addEdge("Kalegana",         "Hapugala",         2);
    addEdge("Hapugala",         "Kalegana",         2);
    addEdge("Kahaduwawaththa",  "Galle_Hospital",   3);
    addEdge("Galle_Hospital",   "Kahaduwawaththa",  3);

    // Hapugala area delivery zones
    addEdge("Hapugala",              "Engineering_Faculty", 1);
    addEdge("Engineering_Faculty",   "Hapugala",            1);
    addEdge("Hapugala",              "Wakwella",            2);
    addEdge("Wakwella",              "Hapugala",            2);

    // Galle Hospital area delivery zones
    addEdge("Galle_Hospital",                "Karapitiya_Medical_Faculty", 2);
    addEdge("Karapitiya_Medical_Faculty",    "Galle_Hospital",             2);
    addEdge("Galle_Hospital",                "Allied_Health_Campus",       1);
    addEdge("Allied_Health_Campus",          "Galle_Hospital",             1);

    // Southern coastal route to Hikkaduwa
    addEdge("Karapitiya_Medical_Faculty",    "Hikkaduwa",                  4);
    addEdge("Hikkaduwa",                     "Karapitiya_Medical_Faculty", 4);

    // Cross connection (Wakwella shortcut to Hikkaduwa via coastal road)
    addEdge("Wakwella",   "Hikkaduwa",  6);
    addEdge("Hikkaduwa",  "Wakwella",   6);

    cout << "[Graph] Galle city map loaded: " << nodes.size() << " locations\n";
}

// return all delivery zones (excludes internal road nodes)
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

// Dijkstra's algorithm - classic O(V^2) version using arrays
pair<int, string> Graph::shortestPath(string dest) {
    int n = nodes.size();

    int startIdx = findNodeIndex("Galle_Restaurant");
    int destIdx  = findNodeIndex(dest);

    if (startIdx == -1 || destIdx == -1) {
        return {-1, "Location not found in city map"};
    }

    vector<int>  dist(n, INT_MAX);
    vector<bool> visited(n, false);
    vector<int>  prev(n, -1);

    dist[startIdx] = 0;

    // main Dijkstra loop
    for (int count = 0; count < n - 1; count++) {
        // find unvisited node with smallest distance
        int minDist = INT_MAX;
        int u = -1;
        for (int i = 0; i < n; i++) {
            if (!visited[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }

        if (u == -1) break;
        visited[u] = true;

        // relax all edges from u
        for (Edge& edge : nodes[u].edges) {
            int v = findNodeIndex(edge.destination);
            if (v != -1 && !visited[v] && dist[u] != INT_MAX) {
                int newDist = dist[u] + edge.weight;
                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    prev[v] = u;
                }
            }
        }
    }

    if (dist[destIdx] == INT_MAX) {
        return {-1, "No path found"};
    }

    // reconstruct path by backtracking through prev[]
    vector<string> path;
    int cur = destIdx;
    while (cur != -1) {
        path.push_back(nodes[cur].name);
        cur = prev[cur];
    }
    reverse(path.begin(), path.end());

    string pathStr = "";
    for (int i = 0; i < (int)path.size(); i++) {
        pathStr += path[i];
        if (i < (int)path.size() - 1) pathStr += " -> ";
    }

    return {dist[destIdx], pathStr};
}

int Graph::nodeCount() {
    return (int)nodes.size();
}