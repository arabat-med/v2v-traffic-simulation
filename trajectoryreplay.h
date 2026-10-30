#ifndef TRAJECTORYREPLAY_H
#define TRAJECTORYREPLAY_H

#include <QString>
#include <vector>

struct ObservedVehicle {
    long long id;
    double lat, lon, speed;
};

struct ObservedFrame {
    double time;
    std::vector<ObservedVehicle> vehicles;
};

class TrajectoryReplay {
public:
    bool load(const QString& path, QString& error);
    int frameAt(double time) const;
    std::vector<ObservedFrame> frames;
    double centerLat = 0, centerLon = 0;
    double radius = 200;
    bool synthetic = false;
};

#endif
