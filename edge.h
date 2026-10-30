#ifndef EDGE_H
#define EDGE_H

#include <string>

struct Node;

struct Edge {
    Node* from;
    Node* to;
    double length;
    std::string road_type;
    double max_speed;
    bool one_way;

    Edge(Node* from, Node* to, double length, const std::string& road_type,
         double max_speed = 50.0, bool one_way = false);

    double getTravelTime() const;
};

#endif
