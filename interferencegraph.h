#ifndef INTERFERENCEGRAPH_H
#define INTERFERENCEGRAPH_H

#include <QObject>
#include <QAbstractListModel>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "vehicle.h"

struct InterferenceEdge {
    long long vehicleId1;
    long long vehicleId2;
    double lat1, lon1;
    double lat2, lon2;
    double distance;
    double strength;
};

struct GridCell {
    std::vector<Vehicle*> vehicles;
};

class InterferenceEdgeModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int edgeCount READ rowCount NOTIFY edgeCountChanged)

public:
    enum EdgeRoles {
        Vehicle1IdRole = Qt::UserRole + 1,
        Vehicle2IdRole,
        Lat1Role,
        Lon1Role,
        Lat2Role,
        Lon2Role,
        DistanceRole,
        StrengthRole
    };

    explicit InterferenceEdgeModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void updateEdges(const std::vector<InterferenceEdge>& edges);
    void updatePositions(const std::unordered_map<long long, Vehicle*>& vehicleMap);
    std::vector<InterferenceEdge>& edges() { return m_edges; }

signals:
    void edgeCountChanged();

private:
    std::vector<InterferenceEdge> m_edges;
};

class InterferenceGraph : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int nodeCount READ nodeCount NOTIFY graphChanged)
    Q_PROPERTY(int edgeCount READ edgeCount NOTIFY graphChanged)
    Q_PROPERTY(double averageDegree READ averageDegree NOTIFY graphChanged)
    Q_PROPERTY(int maxDegree READ maxDegree NOTIFY graphChanged)
    Q_PROPERTY(InterferenceEdgeModel* edgeModel READ edgeModel CONSTANT)

public:
    explicit InterferenceGraph(QObject *parent = nullptr);

    void updateGraph(const std::vector<Vehicle*>& vehicles);
    void updateEdgePositions(const std::vector<Vehicle*>& vehicles);

    int nodeCount() const { return m_nodeCount; }
    int edgeCount() const { return static_cast<int>(m_edges.size()); }
    double averageDegree() const;
    int maxDegree() const;

    InterferenceEdgeModel* edgeModel() { return &m_edgeModel; }

    static double calculateDistance(double lat1, double lon1, double lat2, double lon2);

signals:
    void graphChanged();

private:
    void buildSpatialGrid(const std::vector<Vehicle*>& vehicles);
    std::pair<int, int> getCellIndex(double lat, double lon) const;

    int m_nodeCount;
    std::vector<InterferenceEdge> m_edges;
    std::unordered_map<long long, int> m_degrees;
    InterferenceEdgeModel m_edgeModel;

    std::unordered_map<long long, GridCell> m_grid;
    double m_cellSize;
    double m_minLat, m_maxLat, m_minLon, m_maxLon;
    static constexpr int MAX_EDGES_DISPLAY = 5000;
};

#endif
