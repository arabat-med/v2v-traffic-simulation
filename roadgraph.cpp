#include "roadgraph.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <random>
#include <ctime>
#include <algorithm>
#include <unordered_map>
#include <cmath>

#define QT_NO_CAST_FROM_ASCII
#define QT_NO_CAST_TO_ASCII

RoadGraph::RoadGraph() {
}

RoadGraph::~RoadGraph() {
    clear();
}

void RoadGraph::clear() {
    for (auto& pair : nodes) {
        delete pair.second;
    }
    for (auto edge : edges) {
        delete edge;
    }
    nodes.clear();
    edges.clear();
    spawnPoints.clear();
}

void RoadGraph::addNode(long long id, double lat, double lon) {
    if (nodes.find(id) == nodes.end()) {
        nodes[id] = new Node(id, lat, lon);
    }
}

void RoadGraph::addEdge(long long from_id, long long to_id, double length,
                        const std::string& road_type, double max_speed, bool one_way) {
    Node* from = getNode(from_id);
    Node* to = getNode(to_id);

    if (from && to) {
        Edge* edge = new Edge(from, to, length, road_type, max_speed, one_way);
        edges.push_back(edge);
        from->edges.push_back(edge);

        if (!one_way) {
            Edge* reverse_edge = new Edge(to, from, length, road_type, max_speed, false);
            edges.push_back(reverse_edge);
            to->edges.push_back(reverse_edge);
        }
    }
}

bool RoadGraph::loadFromOSM(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Cannot open OSM file: " << filename << std::endl;
        return false;
    }

    return loadFromOSM(file);
}

bool RoadGraph::loadFromOSM(std::istream& file) {
    clear();
    try {
        std::unordered_map<long long, std::pair<double, double>> tempNodes;

        std::string line;
        bool in_way = false;
        bool is_highway = false;
        std::vector<long long> current_way_nodes;
        std::string current_road_type = "";
        bool current_one_way = false;
        double current_max_speed = 50.0;

        int way_count = 0;

        while (std::getline(file, line)) {
            if (line.find("<node id=") != std::string::npos) {
                long long id;
                double lat, lon;

                size_t id_pos = line.find("id=\"");
                size_t lat_pos = line.find("lat=\"");
                size_t lon_pos = line.find("lon=\"");

                if (id_pos != std::string::npos && lat_pos != std::string::npos && lon_pos != std::string::npos) {
                    id = std::stoll(line.substr(id_pos + 4, line.find("\"", id_pos + 4) - (id_pos + 4)));
                    lat = std::stod(line.substr(lat_pos + 5, line.find("\"", lat_pos + 5) - (lat_pos + 5)));
                    lon = std::stod(line.substr(lon_pos + 5, line.find("\"", lon_pos + 5) - (lon_pos + 5)));
                    tempNodes[id] = std::make_pair(lat, lon);
                }
            }
        }

        std::cout << "  Pass 1: Found " << tempNodes.size() << " total nodes" << std::endl;

        file.clear();
        file.seekg(0);

        while (std::getline(file, line)) {
            if (line.find("<way id=") != std::string::npos) {
                in_way = true;
                is_highway = false;
                current_way_nodes.clear();
                current_road_type = "";
                current_one_way = false;
                current_max_speed = 50.0;
            }
            else if (line.find("</way>") != std::string::npos) {
                if (in_way && is_highway && !current_road_type.empty() && current_way_nodes.size() >= 2) {
                    for (long long nodeId : current_way_nodes) {
                        if (tempNodes.count(nodeId) && nodes.find(nodeId) == nodes.end()) {
                            addNode(nodeId, tempNodes[nodeId].first, tempNodes[nodeId].second);
                        }
                    }

                    for (size_t i = 0; i < current_way_nodes.size() - 1; i++) {
                        long long from = current_way_nodes[i];
                        long long to = current_way_nodes[i + 1];

                        if (nodes.count(from) && nodes.count(to)) {
                            double dist = calculateDistance(
                                nodes[from]->lat, nodes[from]->lon,
                                nodes[to]->lat, nodes[to]->lon
                                );
                            addEdge(from, to, dist, current_road_type, current_max_speed, current_one_way);
                        }
                    }
                    way_count++;
                }
                in_way = false;
                is_highway = false;
            }
            else if (in_way && line.find("<nd ref=") != std::string::npos) {
                size_t ref_pos = line.find("ref=\"");
                if (ref_pos != std::string::npos) {
                    long long node_id = std::stoll(line.substr(ref_pos + 5, line.find("\"", ref_pos + 5) - (ref_pos + 5)));
                    current_way_nodes.push_back(node_id);
                }
            }
            else if (in_way && line.find("k=\"highway\"") != std::string::npos) {
                size_t value_pos = line.find("v=\"");
                if (value_pos != std::string::npos) {
                    std::string value = line.substr(value_pos + 3, line.find("\"", value_pos + 3) - (value_pos + 3));

                    bool isDrivableRoad = (value == "motorway" || value == "trunk" ||
                                          value == "primary" || value == "secondary" ||
                                          value == "tertiary" || value == "residential" ||
                                          value == "unclassified" || value == "service" ||
                                          value == "motorway_link" || value == "trunk_link" ||
                                          value == "primary_link" || value == "secondary_link" ||
                                          value == "tertiary_link" || value == "living_street");

                    if (isDrivableRoad) {
                        is_highway = true;
                        current_road_type = value;

                        if (value == "motorway" || value == "motorway_link") current_max_speed = 110.0;
                        else if (value == "trunk" || value == "trunk_link") current_max_speed = 90.0;
                        else if (value == "primary" || value == "primary_link") current_max_speed = 70.0;
                        else if (value == "secondary" || value == "secondary_link") current_max_speed = 50.0;
                        else if (value == "tertiary"   || value == "tertiary_link") current_max_speed = 40.0;
                        else if (value == "living_street") current_max_speed = 20.0;
                        else current_max_speed = 30.0;
                    }
                }
            }
            else if (in_way && line.find("k=\"oneway\"") != std::string::npos) {
                size_t value_pos = line.find("v=\"");
                if (value_pos != std::string::npos) {
                    std::string value = line.substr(value_pos + 3, line.find("\"", value_pos + 3) - (value_pos + 3));
                    current_one_way = (value == "yes" || value == "1");
                }
            }
        }

        std::cout << "  Pass 2 Complete - Roads Only!" << std::endl;
        std::cout << "  Road nodes: " << nodes.size() << " (filtered from " << tempNodes.size() << " total)" << std::endl;
        std::cout << "  Road ways: " << way_count << std::endl;
        std::cout << "  Road edges: " << edges.size() << std::endl;

        identifySpawnPoints();

        if (nodes.empty() || edges.empty()) {
            clear();
            return false;
        }
        return true;
    } catch (const std::exception& error) {
        std::cerr << "Invalid OSM data: " << error.what() << std::endl;
        clear();
        return false;
    }
}

Node* RoadGraph::getNode(long long id) const {
    auto it = nodes.find(id);
    return (it != nodes.end()) ? it->second : nullptr;
}

const std::vector<Edge*>& RoadGraph::getEdges() const {
    return edges;
}

const std::unordered_map<long long, Node*>& RoadGraph::getNodes() const {
    return nodes;
}

double RoadGraph::calculateDistance(double lat1, double lon1, double lat2, double lon2) const {
    const double PI = 3.14159265358979323846;
    const double R = 6371000.0;
    double dLat = (lat2 - lat1) * PI / 180.0;
    double dLon = (lon2 - lon1) * PI / 180.0;
    double a = sin(dLat/2) * sin(dLat/2) +
               cos(lat1 * PI / 180.0) * cos(lat2 * PI / 180.0) *
                   sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    return R * c;
}

void RoadGraph::printStatistics() const {
    std::cout << "\n========== Road Graph Statistics ==========" << std::endl;
    std::cout << "Nodes: " << nodes.size() << std::endl;
    std::cout << "Edges: " << edges.size() << std::endl;

    std::unordered_map<std::string, int> roadTypeCounts;
    double totalLength = 0.0;
    int oneWayCount = 0;

    for (const auto& edge : edges) {
        roadTypeCounts[edge->road_type]++;
        totalLength += edge->length;
        if (edge->one_way) oneWayCount++;
    }

    std::cout << "\nRoad statistics:" << std::endl;
    std::cout << "  Total length: " << (totalLength / 1000.0) << " km" << std::endl;
    std::cout << "  One-way roads: " << oneWayCount << std::endl;

    std::cout << "\nRoad types:" << std::endl;
    for (const auto& pair : roadTypeCounts) {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }

    int minDegree = 1000, maxDegree = 0;
    double avgDegree = 0.0;

    for (const auto& pair : nodes) {
        int degree = pair.second->edges.size();
        minDegree = std::min(minDegree, degree);
        maxDegree = std::max(maxDegree, degree);
        avgDegree += degree;
    }
    if (!nodes.empty()) {
        avgDegree /= nodes.size();
    }

    std::cout << "\nNode connectivity:" << std::endl;
    std::cout << "  Min degree: " << minDegree << std::endl;
    std::cout << "  Max degree: " << maxDegree << std::endl;
    std::cout << "  Avg degree: " << avgDegree << std::endl;

    std::cout << "==========================================" << std::endl;
}

std::vector<Node*> RoadGraph::getRandomNodes(int count) const {
    std::vector<Node*> result;
    if (nodes.empty()) return result;

    std::vector<Node*> all_nodes;
    for (const auto& pair : nodes) {
        all_nodes.push_back(pair.second);
    }

    std::shuffle(all_nodes.begin(), all_nodes.end(), std::default_random_engine(static_cast<unsigned>(std::time(nullptr))));

    if (count > all_nodes.size()) count = all_nodes.size();
    result.assign(all_nodes.begin(), all_nodes.begin() + count);

    return result;
}

void RoadGraph::identifySpawnPoints() {
    spawnPoints.clear();

    if (nodes.empty()) return;

    double minLat = 1000.0, maxLat = -1000.0;
    double minLon = 1000.0, maxLon = -1000.0;

    for (const auto& pair : nodes) {
        minLat = std::min(minLat, pair.second->lat);
        maxLat = std::max(maxLat, pair.second->lat);
        minLon = std::min(minLon, pair.second->lon);
        maxLon = std::max(maxLon, pair.second->lon);
    }

    double centerLat = (minLat + maxLat) / 2.0;
    double centerLon = (minLon + maxLon) / 2.0;

    for (const auto& pair : nodes) {
        Node* node = pair.second;

        if (node->edges.empty()) continue;

        int connectionCount = node->edges.size();

        std::unordered_map<std::string, int> roadTypes;
        for (Edge* edge : node->edges) {
            roadTypes[edge->road_type]++;
        }

        bool isSpawnPoint = false;
        int priority = 1;

        if (connectionCount >= 4) {
            isSpawnPoint = true;
            priority = 3;
        } else if (connectionCount >= 3 && (roadTypes.count("motorway") || roadTypes.count("primary"))) {
            isSpawnPoint = true;
            priority = 2;
        } else if (connectionCount >= 3) {
            isSpawnPoint = true;
            priority = 1;
        }

        if (isSpawnPoint) {
            std::string zone;
            bool north = node->lat > centerLat;
            bool east = node->lon > centerLon;

            if (north && east) zone = "northeast";
            else if (north && !east) zone = "northwest";
            else if (!north && east) zone = "southeast";
            else zone = "southwest";

            addSpawnPoint(node, zone, priority);
        }
    }

    std::cout << "Identified " << spawnPoints.size() << " spawn points across zones" << std::endl;
}

const std::vector<SpawnPoint>& RoadGraph::getSpawnPoints() const {
    return spawnPoints;
}

std::vector<Node*> RoadGraph::getNodesInZone(const std::string& zone) const {
    std::vector<Node*> result;

    double minLat = 1000.0, maxLat = -1000.0;
    double minLon = 1000.0, maxLon = -1000.0;

    for (const auto& pair : nodes) {
        if (!pair.second->edges.empty()) {
            minLat = std::min(minLat, pair.second->lat);
            maxLat = std::max(maxLat, pair.second->lat);
            minLon = std::min(minLon, pair.second->lon);
            maxLon = std::max(maxLon, pair.second->lon);
        }
    }

    double centerLat = (minLat + maxLat) / 2.0;
    double centerLon = (minLon + maxLon) / 2.0;

    for (const auto& pair : nodes) {
        Node* node = pair.second;
        if (node->edges.empty()) continue;

        bool north = node->lat > centerLat;
        bool east = node->lon > centerLon;

        std::string nodeZone;
        if (north && east) nodeZone = "northeast";
        else if (north && !east) nodeZone = "northwest";
        else if (!north && east) nodeZone = "southeast";
        else nodeZone = "southwest";

        if (nodeZone == zone) {
            result.push_back(node);
        }
    }

    return result;
}

void RoadGraph::addSpawnPoint(Node* node, const std::string& zone, int priority) {
    if (node) {
        spawnPoints.push_back(SpawnPoint(node, zone, priority));
    }
}
