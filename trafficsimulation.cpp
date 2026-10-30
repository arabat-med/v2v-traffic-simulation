#include "trafficsimulation.h"
#include <QVariantMap>
#include <algorithm>
#include <random>
#include <iostream>

TrafficSimulation::TrafficSimulation(RoadGraph* graph, QObject *parent)
    : QObject(parent), m_graph(graph), m_running(false),
    m_timeSpeed(1.0), m_nextVehicleId(1), m_showInterferenceGraph(true)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TrafficSimulation::updateSimulation);
    m_timer->setTimerType(Qt::PreciseTimer);

    if (m_graph) {
        for (const auto& pair : m_graph->getNodes()) {
            if (pair.second && !pair.second->edges.empty()) {
                m_allNodes.push_back(pair.second);
            }
        }

        m_spawnPoints = m_graph->getSpawnPoints();

        std::cout << "Traffic simulation initialized:" << std::endl;
        std::cout << "  - " << m_allNodes.size() << " nodes with edges available" << std::endl;
        std::cout << "  - " << m_spawnPoints.size() << " spawn points identified" << std::endl;
    }

    if (!m_spawnPoints.empty() || !m_allNodes.empty()) {
        initializeVehicles(20);
        std::cout << "  - " << m_vehicles.size() << " vehicles created" << std::endl;
    } else {
        std::cout << "ERROR: No nodes or spawn points available for vehicle placement!" << std::endl;
    }
}

TrafficSimulation::~TrafficSimulation()
{
    clearVehicles();
}

void TrafficSimulation::start()
{
    if (!m_running) {
        m_running = true;
        int interval = 16;
        if (m_vehicles.size() > 1000) interval = 33;
        else if (m_vehicles.size() > 500) interval = 25;

        if (m_showInterferenceGraph) {
            m_interferenceGraph.updateGraph(m_vehicles);
        }

        m_timer->start(interval);
        std::cout << "Simulation started with " << m_vehicles.size() << " vehicles (interval: " << interval << "ms)" << std::endl;
        emit runningChanged();
    }
}

void TrafficSimulation::pause()
{
    if (m_running) {
        m_running = false;
        m_timer->stop();
        emit runningChanged();
    }
}

void TrafficSimulation::reset()
{
    pause();
    clearVehicles();
    initializeVehicles(20);
    emit vehicleCountChanged();
}

void TrafficSimulation::setTimeSpeed(double speed)
{
    if (speed != m_timeSpeed && speed > 0 && speed <= 10.0) {
        m_timeSpeed = speed;
        emit timeSpeedChanged();
    }
}

void TrafficSimulation::addVehicles(int count)
{
    initializeVehicles(count);
    emit vehicleCountChanged();
}

void TrafficSimulation::setVehicleCount(int count)
{
    if (count < 0) count = 0;
    if (count > 2000) count = 2000;

    int currentCount = m_vehicles.size();

    if (count > currentCount) {
        initializeVehicles(count - currentCount);
        m_vehicleModel.setVehicles(m_vehicles);
    } else if (count < currentCount) {
        int toRemove = currentCount - count;
        for (int i = 0; i < toRemove && !m_vehicles.empty(); i++) {
            Vehicle* vehicle = m_vehicles.back();
            m_vehicles.pop_back();
            delete vehicle;
        }
        m_vehicleModel.setVehicles(m_vehicles);
    }

    if (m_running) {
        int interval = 16;
        if (m_vehicles.size() > 1000) interval = 33;
        else if (m_vehicles.size() > 500) interval = 25;
        m_timer->start(interval);
    }

    emit vehicleCountChanged();
    std::cout << "Vehicle count set to: " << m_vehicles.size() << std::endl;
}

void TrafficSimulation::updateSimulation()
{
    if (!m_running) return;

    double deltaTime = 0.016 * m_timeSpeed;

    int stoppedVehicles = 0;
    int movingVehicles = 0;

    for (Vehicle* vehicle : m_vehicles) {
        vehicle->update(deltaTime);

        if (!vehicle->currentEdge && !m_allNodes.empty()) {
            if (!m_spawnPoints.empty() && m_graph) {
                Node* closestSpawn = nullptr;
                std::string closestZone = "";
                double minDist = 1e9;

                for (const auto& sp : m_spawnPoints) {
                    double dist = m_graph->calculateDistance(
                        vehicle->currentLat, vehicle->currentLon,
                        sp.node->lat, sp.node->lon);
                    if (dist < minDist) {
                        minDist = dist;
                        closestSpawn = sp.node;
                        closestZone = sp.zone;
                    }
                }

                if (!closestZone.empty()) {
                    std::vector<Node*> zoneNodes = m_graph->getNodesInZone(closestZone);
                    if (!zoneNodes.empty() && rand() % 100 < 70) {
                        vehicle->assignRandomPath(zoneNodes);
                    } else {
                        vehicle->assignRandomPath(m_allNodes);
                    }
                } else {
                    vehicle->assignRandomPath(m_allNodes);
                }
            } else {
                vehicle->assignRandomPath(m_allNodes);
            }
            stoppedVehicles++;
        } else if (vehicle->currentEdge) {
            movingVehicles++;
        }
    }

    static int debugCounter = 0;
    debugCounter++;
    if (debugCounter >= 300) {
        std::cout << "Moving: " << movingVehicles << " | Stopped/Reassigned: " << stoppedVehicles << std::endl;
        debugCounter = 0;
    }

    m_vehicleModel.updatePositions();

    static int positionCounter = 0;
    positionCounter++;
    if (m_showInterferenceGraph && positionCounter >= 5) {
        m_interferenceGraph.updateEdgePositions(m_vehicles);
        positionCounter = 0;
    }

    static int interferenceCounter = 0;
    interferenceCounter++;

    int updateInterval = 500;
    if (m_vehicles.size() > 1000) updateInterval = 900;  // ~15 sec for 1000+ vehicles
    else if (m_vehicles.size() > 500) updateInterval = 700;  // ~12 sec
    else if (m_vehicles.size() > 200) updateInterval = 600;  // ~10 sec

    if (m_showInterferenceGraph && interferenceCounter >= updateInterval) {
        m_interferenceGraph.updateGraph(m_vehicles);
        interferenceCounter = 0;
    }
}

void TrafficSimulation::setShowInterferenceGraph(bool show)
{
    if (m_showInterferenceGraph != show) {
        m_showInterferenceGraph = show;
        if (show) {
            m_interferenceGraph.updateGraph(m_vehicles);
        }
        emit showInterferenceGraphChanged();
    }
}

void TrafficSimulation::initializeVehicles(int count)
{
    if (m_spawnPoints.empty() && m_allNodes.empty()) {
        std::cout << "ERROR: Cannot initialize vehicles - no spawn points or nodes available!" << std::endl;
        return;
    }

    int successfullyCreated = 0;

    for (int i = 0; i < count; i++) {
        Node* startNode = nullptr;
        std::string assignedZone = "";

        if (!m_spawnPoints.empty()) {
            int totalPriority = 0;
            for (const auto& sp : m_spawnPoints) {
                totalPriority += sp.priority;
            }

            int randomValue = rand() % totalPriority;
            int currentSum = 0;

            for (const auto& sp : m_spawnPoints) {
                currentSum += sp.priority;
                if (randomValue < currentSum) {
                    startNode = sp.node;
                    assignedZone = sp.zone;
                    break;
                }
            }
        }

        if (!startNode && !m_allNodes.empty()) {
            int nodeIndex = rand() % m_allNodes.size();
            startNode = m_allNodes[nodeIndex];
        }

        if (!startNode) continue;

        Vehicle* vehicle = new Vehicle(m_nextVehicleId++, startNode);

        if (!assignedZone.empty()) {
            vehicle->setPreferredZone(assignedZone);
        }

        if (!assignedZone.empty() && m_graph) {
            std::vector<Node*> zoneNodes = m_graph->getNodesInZone(assignedZone);

            if (!zoneNodes.empty()) {
                if (rand() % 100 < 70) {
                    vehicle->assignRandomPath(zoneNodes);
                } else {
                    vehicle->assignRandomPath(m_allNodes);
                }
            } else {
                vehicle->assignRandomPath(m_allNodes);
            }
        } else {
            vehicle->assignRandomPath(m_allNodes);
        }

        if (vehicle->currentEdge) {
            successfullyCreated++;
        }

        m_vehicles.push_back(vehicle);
    }

    std::cout << "Created " << m_vehicles.size() << " vehicles, "
              << successfullyCreated << " have valid paths" << std::endl;

    m_vehicleModel.setVehicles(m_vehicles);
}

void TrafficSimulation::clearVehicles()
{
    for (Vehicle* vehicle : m_vehicles) {
        delete vehicle;
    }
    m_vehicles.clear();
    m_vehicleModel.setVehicles(m_vehicles);
    m_nextVehicleId = 1;
}
