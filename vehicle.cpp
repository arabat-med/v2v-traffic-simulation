#include "vehicle.h"
#include <algorithm>
#include <random>
#include <cmath>

Vehicle::Vehicle(long long id, Node* startNode)
    : id(id), currentNode(startNode), targetNode(nullptr),
    currentEdge(nullptr), progress(0.0), speed(0.0),
    currentLat(startNode->lat), currentLon(startNode->lon),
    pathIndex(0), preferredZone("")
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> radiusDist(100.0, 500.0);
    transmissionRadius = radiusDist(gen);
}

void Vehicle::update(double deltaTime) {
    if (!currentEdge || !targetNode) {
        moveToNextNode();
        return;
    }

    double distance = speed * deltaTime;
    double edgeLength = currentEdge->length;

    if (edgeLength > 0) {
        progress += distance / edgeLength;

        if (progress >= 1.0) {
            currentNode = targetNode;
            currentLat = currentNode->lat;
            currentLon = currentNode->lon;
            progress = 0.0;
            moveToNextNode();
        } else {
            updatePosition();
        }
    } else {
        moveToNextNode();
    }
}

void Vehicle::moveToNextNode() {
    if (path.empty() || pathIndex >= path.size() - 1) {
        currentEdge = nullptr;
        targetNode = nullptr;
        speed = 0.0;
        return;
    }

    pathIndex++;
    targetNode = path[pathIndex];

    currentEdge = nullptr;
    for (Edge* edge : currentNode->edges) {
        if (edge && edge->to == targetNode) {
            currentEdge = edge;
            break;
        }
    }

    if (currentEdge) {
        speed = currentEdge->max_speed * 1000.0 / 3600.0;
        progress = 0.0;
        updatePosition();
    } else {
        speed = 0.0;
        currentEdge = nullptr;
        targetNode = nullptr;
    }
}

void Vehicle::updatePosition() {
    if (!currentNode || !targetNode) return;

    currentLat = currentNode->lat + (targetNode->lat - currentNode->lat) * progress;
    currentLon = currentNode->lon + (targetNode->lon - currentNode->lon) * progress;
}

void Vehicle::assignRandomPath(const std::vector<Node*>& allNodes) {
    if (allNodes.empty() || !currentNode) return;

    path.clear();
    pathIndex = 0;

    path.push_back(currentNode);

    Node* current = currentNode;
    int maxPathLength = 50 + (rand() % 100);
    int attempts = 0;
    int maxAttempts = 200;

    while (path.size() < static_cast<size_t>(maxPathLength) && attempts < maxAttempts) {
        attempts++;

        if (current->edges.empty()) {
            break;
        }

        Edge* bestEdge = nullptr;
        std::vector<Edge*> validEdges;

        for (Edge* edge : current->edges) {
            if (edge && edge->to) {
                if (path.size() >= 2 && edge->to == path[path.size() - 2]) {
                    continue;
                }
                validEdges.push_back(edge);
            }
        }

        if (validEdges.empty()) {
            break;
        }

        int edgeIndex = rand() % validEdges.size();
        bestEdge = validEdges[edgeIndex];

        if (bestEdge && bestEdge->to) {
            current = bestEdge->to;
            path.push_back(current);
            attempts = 0;
        }
    }

    if (path.size() < 2) {
        path.clear();
        currentEdge = nullptr;
        targetNode = nullptr;
        speed = 0.0;
        return;
    }

    pathIndex = 0;
    moveToNextNode();
}

void Vehicle::setPreferredZone(const std::string& zone) {
    preferredZone = zone;
}
