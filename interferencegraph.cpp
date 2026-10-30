#include "interferencegraph.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <set>

const double EARTH_RADIUS_METERS = 6371000.0;
const double PI = 3.14159265358979323846;
const double METERS_PER_DEGREE_LAT = 111000.0;

InterferenceEdgeModel::InterferenceEdgeModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int InterferenceEdgeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(m_edges.size());
}

QVariant InterferenceEdgeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_edges.size()))
        return QVariant();

    const InterferenceEdge& edge = m_edges[index.row()];

    switch (role) {
    case Vehicle1IdRole: return QVariant::fromValue(edge.vehicleId1);
    case Vehicle2IdRole: return QVariant::fromValue(edge.vehicleId2);
    case Lat1Role: return edge.lat1;
    case Lon1Role: return edge.lon1;
    case Lat2Role: return edge.lat2;
    case Lon2Role: return edge.lon2;
    case DistanceRole: return edge.distance;
    case StrengthRole: return edge.strength;
    default: return QVariant();
    }
}

QHash<int, QByteArray> InterferenceEdgeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Vehicle1IdRole] = "vehicleId1";
    roles[Vehicle2IdRole] = "vehicleId2";
    roles[Lat1Role] = "lat1";
    roles[Lon1Role] = "lon1";
    roles[Lat2Role] = "lat2";
    roles[Lon2Role] = "lon2";
    roles[DistanceRole] = "distance";
    roles[StrengthRole] = "strength";
    return roles;
}

void InterferenceEdgeModel::updateEdges(const std::vector<InterferenceEdge>& edges)
{
    beginResetModel();
    m_edges = edges;
    endResetModel();
    emit edgeCountChanged();
}

void InterferenceEdgeModel::updatePositions(const std::unordered_map<long long, Vehicle*>& vehicleMap)
{
    if (m_edges.empty()) return;

    for (auto& edge : m_edges) {
        auto it1 = vehicleMap.find(edge.vehicleId1);
        auto it2 = vehicleMap.find(edge.vehicleId2);

        if (it1 != vehicleMap.end() && it2 != vehicleMap.end()) {
            edge.lat1 = it1->second->currentLat;
            edge.lon1 = it1->second->currentLon;
            edge.lat2 = it2->second->currentLat;
            edge.lon2 = it2->second->currentLon;
        }
    }

    if (!m_edges.empty()) {
        emit dataChanged(index(0), index(static_cast<int>(m_edges.size()) - 1),
                         {Lat1Role, Lon1Role, Lat2Role, Lon2Role});
    }
}

InterferenceGraph::InterferenceGraph(QObject *parent)
    : QObject(parent), m_nodeCount(0), m_cellSize(0.005),
      m_minLat(0), m_maxLat(0), m_minLon(0), m_maxLon(0)
{
}

double InterferenceGraph::calculateDistance(double lat1, double lon1, double lat2, double lon2)
{
    double lat1Rad = lat1 * PI / 180.0;
    double lat2Rad = lat2 * PI / 180.0;
    double deltaLat = (lat2 - lat1) * PI / 180.0;
    double deltaLon = (lon2 - lon1) * PI / 180.0;

    double a = sin(deltaLat / 2) * sin(deltaLat / 2) +
               cos(lat1Rad) * cos(lat2Rad) *
               sin(deltaLon / 2) * sin(deltaLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return EARTH_RADIUS_METERS * c;
}

std::pair<int, int> InterferenceGraph::getCellIndex(double lat, double lon) const
{
    int cellX = static_cast<int>((lat - m_minLat) / m_cellSize);
    int cellY = static_cast<int>((lon - m_minLon) / m_cellSize);
    return {cellX, cellY};
}

void InterferenceGraph::buildSpatialGrid(const std::vector<Vehicle*>& vehicles)
{
    m_grid.clear();

    if (vehicles.empty()) return;

    m_minLat = m_maxLat = vehicles[0]->currentLat;
    m_minLon = m_maxLon = vehicles[0]->currentLon;

    for (const Vehicle* v : vehicles) {
        m_minLat = std::min(m_minLat, v->currentLat);
        m_maxLat = std::max(m_maxLat, v->currentLat);
        m_minLon = std::min(m_minLon, v->currentLon);
        m_maxLon = std::max(m_maxLon, v->currentLon);
    }

    m_minLat -= m_cellSize * 2;
    m_minLon -= m_cellSize * 2;

    for (Vehicle* v : vehicles) {
        auto [cellX, cellY] = getCellIndex(v->currentLat, v->currentLon);
        long long cellKey = static_cast<long long>(cellX) * 100000 + cellY;
        m_grid[cellKey].vehicles.push_back(v);
    }
}

void InterferenceGraph::updateGraph(const std::vector<Vehicle*>& vehicles)
{
    m_edges.clear();
    m_degrees.clear();
    m_nodeCount = static_cast<int>(vehicles.size());

    if (vehicles.empty()) {
        m_edgeModel.updateEdges(m_edges);
        emit graphChanged();
        return;
    }

    for (const Vehicle* v : vehicles) {
        m_degrees[v->id] = 0;
    }

    buildSpatialGrid(vehicles);

    std::set<std::pair<long long, long long>> processedPairs;

    for (Vehicle* v1 : vehicles) {
        auto [cellX, cellY] = getCellIndex(v1->currentLat, v1->currentLon);

        for (int dx = -2; dx <= 2; dx++) {
            for (int dy = -2; dy <= 2; dy++) {
                long long neighborKey = static_cast<long long>(cellX + dx) * 100000 + (cellY + dy);

                auto it = m_grid.find(neighborKey);
                if (it == m_grid.end()) continue;

                for (Vehicle* v2 : it->second.vehicles) {
                    if (v1->id >= v2->id) continue;

                    auto pairKey = std::make_pair(std::min(v1->id, v2->id), std::max(v1->id, v2->id));
                    if (processedPairs.count(pairKey)) continue;
                    processedPairs.insert(pairKey);

                    double distance = calculateDistance(
                        v1->currentLat, v1->currentLon,
                        v2->currentLat, v2->currentLon
                    );

                    double maxRadius = std::max(v1->transmissionRadius, v2->transmissionRadius);

                    if (distance <= maxRadius) {
                        double strength = 1.0 - (distance / maxRadius);
                        strength = std::max(0.0, std::min(1.0, strength));

                        InterferenceEdge edge;
                        edge.vehicleId1 = v1->id;
                        edge.vehicleId2 = v2->id;
                        edge.lat1 = v1->currentLat;
                        edge.lon1 = v1->currentLon;
                        edge.lat2 = v2->currentLat;
                        edge.lon2 = v2->currentLon;
                        edge.distance = distance;
                        edge.strength = strength;

                        m_edges.push_back(edge);

                        m_degrees[v1->id]++;
                        m_degrees[v2->id]++;

                        if (m_edges.size() >= MAX_EDGES_DISPLAY) {
                            goto done;
                        }
                    }
                }
            }
        }
    }

done:
    std::cout << "Interference graph: " << m_edges.size() << " edges for " << vehicles.size() << " vehicles" << std::endl;

    m_edgeModel.updateEdges(m_edges);
    emit graphChanged();
}

void InterferenceGraph::updateEdgePositions(const std::vector<Vehicle*>& vehicles)
{
    std::unordered_map<long long, Vehicle*> vehicleMap;
    for (Vehicle* v : vehicles) {
        vehicleMap[v->id] = v;
    }

    m_edgeModel.updatePositions(vehicleMap);
}

double InterferenceGraph::averageDegree() const
{
    if (m_nodeCount == 0) return 0.0;

    int totalDegree = 0;
    for (const auto& pair : m_degrees) {
        totalDegree += pair.second;
    }
    return static_cast<double>(totalDegree) / m_nodeCount;
}

int InterferenceGraph::maxDegree() const
{
    int maxDeg = 0;
    for (const auto& pair : m_degrees) {
        maxDeg = std::max(maxDeg, pair.second);
    }
    return maxDeg;
}
