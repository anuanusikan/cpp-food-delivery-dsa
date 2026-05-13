#pragma once
#include <string>
#include <vector>
#include <climits>
#include <algorithm>
#include <iostream>

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

    int findNodeIndex(string name) {
        for (int i = 0; i < (int)nodes.size(); i++) {
            if (nodes[i].name == name) {
                return i;
            }
        }

        return -1;
    }

    void addEdge(string from, string to, int weight) {
        int fromIndex = findNodeIndex(from);

        if (fromIndex == -1) {
            GraphNode newNode;
            newNode.name = from;
            nodes.push_back(newNode);
            fromIndex = nodes.size() - 1;
        }

        if (findNodeIndex(to) == -1) {
            GraphNode newNode;
            newNode.name = to;
            nodes.push_back(newNode);
        }

        Edge edge;
        edge.destination = to;
        edge.weight = weight;

        nodes[fromIndex].edges.push_back(edge);
    }

public:
    void init() {
        nodes.clear();

        addEdge("Galle_Restaurant", "Main_Junction", 1);
        addEdge("Main_Junction", "Galle_Restaurant", 1);

        addEdge("Main_Junction", "Kalegana_Junction", 2);
        addEdge("Kalegana_Junction", "Main_Junction", 2);

        addEdge("Main_Junction", "Kahaduwawaththa_Junction", 3);
        addEdge("Kahaduwawaththa_Junction", "Main_Junction", 3);

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

        addEdge("Kalegana_Junction", "Kahaduwawaththa_Junction", 2);
        addEdge("Kahaduwawaththa_Junction", "Kalegana_Junction", 2);

        addEdge("Hapugala_Junction", "Hospital_Junction", 4);
        addEdge("Hospital_Junction", "Hapugala_Junction", 4);

        addEdge("Wakwella_Road_Junction", "Karapitiya_Junction", 5);
        addEdge("Karapitiya_Junction", "Wakwella_Road_Junction", 5);

        addEdge("Karapitiya_Junction", "Coastal_Road_Junction", 3);
        addEdge("Coastal_Road_Junction", "Karapitiya_Junction", 3);

        addEdge("Coastal_Road_Junction", "Hikkaduwa", 4);
        addEdge("Hikkaduwa", "Coastal_Road_Junction", 4);

        addEdge("Wakwella", "Coastal_Road_Junction", 6);
        addEdge("Coastal_Road_Junction", "Wakwella", 6);

        cout << "[Graph] Galle city map loaded with " << nodes.size() << " locations\n";
    }

    void addAddress(string address) {
        if (address.empty()) {
            return;
        }

        if (findNodeIndex(address) != -1) {
            return;
        }

        addEdge(address, "Hapugala", 2);
        addEdge("Hapugala", address, 2);
    }

    void addDynamicUserLocation(string customerNode, string nearestLocation, int distance) {
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

    pair<int, string> shortestPath(string destination) {
        return shortestPath("Galle_Restaurant", destination);
    }

    pair<int, string> shortestPath(string source, string destination) {
        int n = nodes.size();

        int startIndex = findNodeIndex(source);
        int destinationIndex = findNodeIndex(destination);

        if (startIndex == -1 || destinationIndex == -1) {
            return {-1, "Location not found"};
        }

        vector<int> distance(n, INT_MAX);
        vector<bool> visited(n, false);
        vector<int> previous(n, -1);

        distance[startIndex] = 0;

        for (int count = 0; count < n - 1; count++) {
            int minimumDistance = INT_MAX;
            int currentNode = -1;

            for (int i = 0; i < n; i++) {
                if (!visited[i] && distance[i] < minimumDistance) {
                    minimumDistance = distance[i];
                    currentNode = i;
                }
            }

            if (currentNode == -1) {
                break;
            }

            visited[currentNode] = true;

            for (Edge& edge : nodes[currentNode].edges) {
                int nextNode = findNodeIndex(edge.destination);

                if (nextNode != -1 && !visited[nextNode] && distance[currentNode] != INT_MAX) {
                    int newDistance = distance[currentNode] + edge.weight;

                    if (newDistance < distance[nextNode]) {
                        distance[nextNode] = newDistance;
                        previous[nextNode] = currentNode;
                    }
                }
            }
        }

        if (distance[destinationIndex] == INT_MAX) {
            return {-1, "No path found"};
        }

        vector<string> path;
        int current = destinationIndex;

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

        return {distance[destinationIndex], pathText};
    }

    int distanceBetween(string source, string destination) {
        pair<int, string> result = shortestPath(source, destination);

        if (result.first < 0) {
            return INT_MAX / 2;
        }

        return result.first;
    }

    pair<int, string> optimizedMultiStopRoute(vector<string> deliveryLocations) {
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

    vector<string> getDeliveryZones() {
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

    vector<string> getPredefinedLocations() {
        return getDeliveryZones();
    }

    int nodeCount() {
        return nodes.size();
    }
};