#ifndef NODE_H
#define NODE_H

#include <vector>

struct Edge;

struct Node {
    long long id;
    double lat, lon;
    std::vector<Edge*> edges;

    Node(long long id, double lat, double lon);
    double distanceTo(const Node& other) const;
};

#endif
