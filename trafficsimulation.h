#ifndef TRAFFICSIMULATION_H
#define TRAFFICSIMULATION_H

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <QElapsedTimer>
#include <QUrl>
#include <vector>
#include "vehicle.h"
#include "roadgraph.h"
#include "vehiclemodel.h"
#include "interferencegraph.h"
#include "trajectoryreplay.h"

class TrafficSimulation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(double timeSpeed READ timeSpeed WRITE setTimeSpeed NOTIFY timeSpeedChanged)
    Q_PROPERTY(int vehicleCount READ vehicleCount NOTIFY vehicleCountChanged)
    Q_PROPERTY(VehicleModel* vehicleModel READ vehicleModel CONSTANT)
    Q_PROPERTY(InterferenceGraph* interferenceGraph READ interferenceGraph CONSTANT)
    Q_PROPERTY(bool showInterferenceGraph READ showInterferenceGraph WRITE setShowInterferenceGraph NOTIFY showInterferenceGraphChanged)
    Q_PROPERTY(bool replayMode READ replayMode NOTIFY sourceChanged)
    Q_PROPERTY(QString sourceError READ sourceError NOTIFY sourceErrorChanged)
    Q_PROPERTY(bool syntheticReplay READ syntheticReplay NOTIFY sourceChanged)
    Q_PROPERTY(double replayTime READ replayTime NOTIFY replayTimeChanged)
    Q_PROPERTY(double replayDuration READ replayDuration NOTIFY sourceChanged)
    Q_PROPERTY(double replayCenterLat READ replayCenterLat NOTIFY sourceChanged)
    Q_PROPERTY(double replayCenterLon READ replayCenterLon NOTIFY sourceChanged)

public:
    explicit TrafficSimulation(RoadGraph* graph, QObject *parent = nullptr);
    ~TrafficSimulation();

    bool isRunning() const { return m_running; }
    double timeSpeed() const { return m_timeSpeed; }
    int vehicleCount() const { return m_vehicles.size(); }
    VehicleModel* vehicleModel() { return &m_vehicleModel; }
    InterferenceGraph* interferenceGraph() { return &m_interferenceGraph; }
    bool showInterferenceGraph() const { return m_showInterferenceGraph; }
    bool replayMode() const { return m_replayMode; }
    bool syntheticReplay() const { return m_replay.synthetic; }
    QString sourceError() const { return m_sourceError; }
    double replayTime() const { return m_replayTime; }
    double replayDuration() const { return m_replay.frames.empty() ? 0 : m_replay.frames.back().time; }
    double replayCenterLat() const { return m_replay.centerLat; }
    double replayCenterLon() const { return m_replay.centerLon; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void setTimeSpeed(double speed);
    Q_INVOKABLE void setVehicleCount(int count);
    Q_INVOKABLE void addVehicles(int count);
    Q_INVOKABLE void setShowInterferenceGraph(bool show);
    Q_INVOKABLE bool loadTrajectories(const QUrl& file);
    Q_INVOKABLE void useSimulation();

signals:
    void runningChanged();
    void timeSpeedChanged();
    void vehicleCountChanged();
    void showInterferenceGraphChanged();
    void sourceChanged();
    void sourceErrorChanged();
    void replayTimeChanged();

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
    bool m_replayMode = false;
    TrajectoryReplay m_replay;
    QString m_sourceError;
    double m_replayTime = 0;
    int m_replayFrame = -1;
    QElapsedTimer m_replayClock;

    void initializeVehicles(int count);
    void clearVehicles();
    void applyReplayFrame(int index);
};

#endif
