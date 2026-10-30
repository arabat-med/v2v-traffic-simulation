#ifndef ROADGRAPH_H
#define ROADGRAPH_H

#include "node.h"
#include "edge.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <istream>

struct SpawnPoint {
    Node* node;
    std::string zone;
    int priority;

    SpawnPoint(Node* n, const std::string& z, int p = 1)
        : node(n), zone(z), priority(p) {}
};

class RoadGraph {
private:
    std::unordered_map<long long, Node*> nodes;
    std::vector<Edge*> edges;
    std::vector<SpawnPoint> spawnPoints;
    void clear();

public:
    RoadGraph();
    ~RoadGraph();

    void addNode(long long id, double lat, double lon);
    void addEdge(long long from_id, long long to_id, double length,
                 const std::string& road_type, double max_speed = 50.0, bool one_way = false);

    bool loadFromOSM(const std::string& filename);
    bool loadFromOSM(std::istream& input);

    Node* getNode(long long id) const;
    const std::vector<Edge*>& getEdges() const;
    const std::unordered_map<long long, Node*>& getNodes() const;

    double calculateDistance(double lat1, double lon1, double lat2, double lon2) const;
    void printStatistics() const;

    std::vector<Node*> getRandomNodes(int count) const;

    void identifySpawnPoints();
    const std::vector<SpawnPoint>& getSpawnPoints() const;
    std::vector<Node*> getNodesInZone(const std::string& zone) const;
    void addSpawnPoint(Node* node, const std::string& zone, int priority = 1);
};

#endif
