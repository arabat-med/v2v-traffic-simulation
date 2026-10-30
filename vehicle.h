#ifndef VEHICLE_H
#define VEHICLE_H

#include "node.h"
#include "edge.h"
#include <vector>

class Vehicle {
public:
    long long id;
    Node* currentNode;
    Node* targetNode;
    Edge* currentEdge;

    double progress;

    double speed;

    double currentLat;
    double currentLon;

    double transmissionRadius;

    std::vector<Node*> path;
    int pathIndex;
    std::string preferredZone;

    Vehicle(long long id, Node* startNode);

    void update(double deltaTime);

    void assignRandomPath(const std::vector<Node*>& allNodes);

    void setPreferredZone(const std::string& zone);

private:
    void moveToNextNode();
    void updatePosition();
};

#endif
