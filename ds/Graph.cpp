#include "Graph.h"
#include <climits>
#include <algorithm>
#include <cstdlib>

int Graph::findNodeIndex(string name) {
    for (int i = 0; i < (int)nodes.size(); i++) {
        if (nodes[i].name == name) {
            return i;
        }
    }

    return -1;
}

void Graph::addEdge(string from, string to, int weight) {
    int fromIndex = findNodeIndex(from);

    if (fromIndex == -1) {
        GraphNode newNode;
        newNode.name = from;
        nodes.push_back(newNode);
        fromIndex = nodes.size() - 1;
    }

    Edge edge;
    edge.destination = to;
    edge.weight = weight;

    nodes[fromIndex].edges.push_back(edge);
}

void Graph::init() {
    addEdge("Restaurant", "Junction_A", 4);
    addEdge("Junction_A", "Restaurant", 4);

    addEdge("Restaurant", "Junction_B", 2);
    addEdge("Junction_B", "Restaurant", 2);

    addEdge("Junction_A", "Main_Road", 5);
    addEdge("Main_Road", "Junction_A", 5);

    addEdge("Junction_B", "Junction_A", 1);
    addEdge("Junction_A", "Junction_B", 1);

    addEdge("Junction_B", "Main_Road", 8);
    addEdge("Main_Road", "Junction_B", 8);
}

void Graph::addAddress(string addr) {
    if (addr.empty()) {
        return;
    }

    if (findNodeIndex(addr) != -1) {
        return;
    }

    int randomDistance = (rand() % 5) + 1;

    addEdge(addr, "Main_Road", randomDistance);
    addEdge("Main_Road", addr, randomDistance);
}

pair<int, string> Graph::shortestPath(string dest) {
    int n = nodes.size();

    int startIndex = findNodeIndex("Restaurant");
    int destIndex = findNodeIndex(dest);

    if (startIndex == -1 || destIndex == -1) {
        return {0, "Location not mapped"};
    }

    vector<int> distance(n, INT_MAX);
    vector<bool> visited(n, false);
    vector<int> previous(n, -1);

    distance[startIndex] = 0;

    for (int count = 0; count < n - 1; count++) {
        int minDistance = INT_MAX;
        int u = -1;

        for (int i = 0; i < n; i++) {
            if (!visited[i] && distance[i] < minDistance) {
                minDistance = distance[i];
                u = i;
            }
        }

        if (u == -1) {
            break;
        }

        visited[u] = true;

        for (Edge edge : nodes[u].edges) {
            int v = findNodeIndex(edge.destination);

            if (v != -1 && !visited[v] && distance[u] != INT_MAX) {
                if (distance[u] + edge.weight < distance[v]) {
                    distance[v] = distance[u] + edge.weight;
                    previous[v] = u;
                }
            }
        }
    }

    if (distance[destIndex] == INT_MAX) {
        return {0, "No path found"};
    }

    vector<string> path;
    int current = destIndex;

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

    return {distance[destIndex], pathText};
}

int Graph::nodeCount() {
    return nodes.size();
}