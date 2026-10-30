#ifndef VEHICLEMODEL_H
#define VEHICLEMODEL_H

#include <QAbstractListModel>
#include <QTimer>
#include <vector>
#include "vehicle.h"

class VehicleModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum VehicleRoles {
        IdRole = Qt::UserRole + 1,
        LatRole,
        LonRole,
        SpeedRole,
        TransmissionRadiusRole
    };

    explicit VehicleModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setVehicles(const std::vector<Vehicle*>& vehicles);
    void updatePositions();

signals:
    void countChanged();

private:
    std::vector<Vehicle*> m_vehicles;
};

#endif
