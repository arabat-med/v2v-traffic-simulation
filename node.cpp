#include "node.h"
#include "edge.h"
#include <cmath>

const double PI = 3.14159265358979323846;
const double EARTH_RADIUS = 6371000.0;

Node::Node(long long id, double lat, double lon)
    : id(id), lat(lat), lon(lon) {
}

double Node::distanceTo(const Node& other) const {
    double dLat = (other.lat - lat) * PI / 180.0;
    double dLon = (other.lon - lon) * PI / 180.0;
    double a = sin(dLat/2) * sin(dLat/2) +
               cos(lat * PI / 180.0) * cos(other.lat * PI / 180.0) *
                   sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    return EARTH_RADIUS * c;
}
