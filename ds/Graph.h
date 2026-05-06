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
    pair<int, string> shortestPath(string dest);
    int nodeCount();
    vector<string> getDeliveryZones();  // returns all customer-facing zones
};