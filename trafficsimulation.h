#ifndef TRAFFICSIMULATION_H
#define TRAFFICSIMULATION_H

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <vector>
#include "vehicle.h"
#include "roadgraph.h"
#include "vehiclemodel.h"
#include "interferencegraph.h"

class TrafficSimulation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(double timeSpeed READ timeSpeed WRITE setTimeSpeed NOTIFY timeSpeedChanged)
    Q_PROPERTY(int vehicleCount READ vehicleCount NOTIFY vehicleCountChanged)
    Q_PROPERTY(VehicleModel* vehicleModel READ vehicleModel CONSTANT)
    Q_PROPERTY(InterferenceGraph* interferenceGraph READ interferenceGraph CONSTANT)
    Q_PROPERTY(bool showInterferenceGraph READ showInterferenceGraph WRITE setShowInterferenceGraph NOTIFY showInterferenceGraphChanged)

public:
    explicit TrafficSimulation(RoadGraph* graph, QObject *parent = nullptr);
    ~TrafficSimulation();

    bool isRunning() const { return m_running; }
    double timeSpeed() const { return m_timeSpeed; }
    int vehicleCount() const { return m_vehicles.size(); }
    VehicleModel* vehicleModel() { return &m_vehicleModel; }
    InterferenceGraph* interferenceGraph() { return &m_interferenceGraph; }
    bool showInterferenceGraph() const { return m_showInterferenceGraph; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void setTimeSpeed(double speed);
    Q_INVOKABLE void setVehicleCount(int count);
    Q_INVOKABLE void addVehicles(int count);
    Q_INVOKABLE void setShowInterferenceGraph(bool show);

signals:
    void runningChanged();
    void timeSpeedChanged();
    void vehicleCountChanged();
    void showInterferenceGraphChanged();

private slots:
    void updateSimulation();

private:
    RoadGraph* m_graph;
    std::vector<Vehicle*> m_vehicles;
    std::vector<Node*> m_allNodes;
    std::vector<SpawnPoint> m_spawnPoints;

    QTimer* m_timer;
    bool m_running;
    double m_timeSpeed;
    long long m_nextVehicleId;
    VehicleModel m_vehicleModel;
    InterferenceGraph m_interferenceGraph;
    bool m_showInterferenceGraph;

    void initializeVehicles(int count);
    void clearVehicles();
};

#endif
