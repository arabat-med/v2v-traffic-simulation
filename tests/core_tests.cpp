#include <QCoreApplication>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "roadgraph.h"
#include "vehicle.h"
#include "interferencegraph.h"

static void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    try {
        require(argc == 2, "demo path is required");
        RoadGraph demo;
        require(demo.loadFromOSM(std::string(argv[1])), "real demo map must load");
        require(demo.getNodes().size() == 1887, "demo road node count changed");
        require(demo.getEdges().size() == 3757, "demo directed edge count changed");
        require(!demo.getSpawnPoints().empty(), "demo must provide spawn points");

        std::istringstream empty("<osm></osm>\n");
        require(!demo.loadFromOSM(empty), "empty road graph must be rejected");
        require(demo.getNodes().empty(), "reloading must clear the previous graph");
        std::istringstream malformed("<node id=\"bad\" lat=\"47\" lon=\"7\"/>\n");
        require(!demo.loadFromOSM(malformed), "invalid numeric values must be rejected");
        require(!demo.loadFromOSM(std::string("this-map-does-not-exist.osm")), "missing file must be rejected");

        RoadGraph road;
        road.addNode(1, 47.77, 7.32);
        road.addNode(2, 47.771, 7.32);
        road.addNode(3, 47.772, 7.32);
        road.addEdge(1, 2, 100.0, "residential", 36.0, true);
        road.addEdge(2, 3, 100.0, "residential", 36.0, true);
        require(road.getEdges().size() == 2 && road.getNode(3)->edges.empty(), "one-way edges must stay directed");
        Vehicle vehicle(1, road.getNode(1));
        vehicle.assignRandomPath({road.getNode(1), road.getNode(2), road.getNode(3)});
        vehicle.update(1.0);
        require(std::abs(vehicle.speed - 10.0) < 1e-9, "km/h conversion must produce m/s");
        require(std::abs(vehicle.progress - 0.1) < 1e-9, "position must advance by distance over edge length");

        Vehicle other(2, road.getNode(1));
        vehicle.currentLat = other.currentLat = 47.77;
        vehicle.currentLon = other.currentLon = 7.32;
        vehicle.transmissionRadius = 100.0;
        other.transmissionRadius = 200.0;
        InterferenceGraph graph;
        graph.updateGraph({&vehicle, &other});
        require(graph.nodeCount() == 2 && graph.edgeCount() == 1, "co-located vehicles must connect");
        require(graph.averageDegree() == 1.0 && graph.maxDegree() == 1, "degree statistics must match the link");
        require(graph.edgeModel()->edges().front().strength == 1.0, "zero distance must give unit geometric strength");
        other.currentLon += 0.02;
        graph.updateGraph({&vehicle, &other});
        require(graph.edgeCount() == 0, "out-of-range vehicles must not connect");
        graph.updateGraph({});
        require(graph.nodeCount() == 0 && graph.edgeCount() == 0 && graph.averageDegree() == 0.0, "empty graph must reset statistics");
        std::cout << "CORE_TESTS_OK: OSM loading, directed roads, movement and range graph" << std::endl;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "CORE_TESTS_FAILED: " << error.what() << std::endl;
        return 1;
    }
}
