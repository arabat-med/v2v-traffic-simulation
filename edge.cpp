#include "edge.h"
#include "node.h"

Edge::Edge(Node* from, Node* to, double length, const std::string& road_type,
           double max_speed, bool one_way)
    : from(from), to(to), length(length), road_type(road_type),
    max_speed(max_speed), one_way(one_way) {
}

double Edge::getTravelTime() const {
    return length / (max_speed * 1000.0 / 3600.0);
}
