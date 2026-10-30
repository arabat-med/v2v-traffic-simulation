#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QTemporaryFile>
#include <QTimer>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "roadgraph.h"
#include "vehicle.h"
#include "interferencegraph.h"
#include "trafficsimulation.h"

static void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    try {
        require(argc == 3, "demo and synthetic trajectory fixture paths are required");
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

        QString replayError;
        TrajectoryReplay replay;
        require(replay.load(QString::fromLocal8Bit(argv[2]), replayError), "synthetic replay fixture must load");
        require(replay.synthetic && replay.frames.size() == 3 && replay.frames[1].vehicles.empty(), "preserve synthetic status and empty frames");
        require(replay.frameAt(0.05) == 0 && replay.frameAt(0.1) == 1 && replay.frameAt(5.0) == 2, "timestamp selection must include empty and final frames");
        QTemporaryFile invalid;
        require(invalid.open(), "temporary file must open");
        invalid.write("{\"type\":\"v2v_detections\",\"schema_version\":1}\n"); invalid.flush();
        require(!replay.load(invalid.fileName(), replayError) && replay.frames.size() == 3, "reject uncalibrated detections without losing the previous replay");

        TrafficSimulation simulation(&road);
        require(simulation.loadTrajectories(QUrl::fromLocalFile(QString::fromLocal8Bit(argv[2]))), "simulation must import calibrated trajectories");
        require(simulation.replayMode() && simulation.vehicleCount() == 2 && simulation.interferenceGraph()->edgeCount() == 1, "import must feed the existing Qt model and geometric range graph");
        simulation.setVehicleCount(100);
        require(simulation.vehicleCount() == 2, "random population controls must not alter observations");
        simulation.start();
        QEventLoop loop;
        QTimer::singleShot(350, &loop, &QEventLoop::quit); loop.exec();
        require(!simulation.isRunning() && simulation.vehicleCount() == 1, "playback must use elapsed time and stop at the final observed frame");
        require(simulation.interferenceGraph()->nodeCount() == 1 && simulation.interferenceGraph()->edgeCount() == 0, "departed detections must leave the graph");
        simulation.reset();
        require(simulation.vehicleCount() == 2 && simulation.replayTime() == 0, "reset must rewind the observation sequence");
        simulation.useSimulation();
        require(!simulation.replayMode() && simulation.vehicleCount() == 20, "random simulation must remain available after replay");
        std::cout << "CORE_TESTS_OK: OSM loading, directed roads, movement and range graph" << std::endl;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "CORE_TESTS_FAILED: " << error.what() << std::endl;
        return 1;
    }
}
