#include "vehiclemodel.h"

VehicleModel::VehicleModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int VehicleModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_vehicles.size());
}

QVariant VehicleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_vehicles.size()))
        return QVariant();

    const Vehicle* vehicle = m_vehicles[index.row()];

    switch (role) {
    case IdRole:
        return QVariant::fromValue(static_cast<qint64>(vehicle->id));
    case LatRole:
        return vehicle->currentLat;
    case LonRole:
        return vehicle->currentLon;
    case SpeedRole:
        return vehicle->speed;
    case TransmissionRadiusRole:
        return vehicle->transmissionRadius;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> VehicleModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "vehicleId";
    roles[LatRole] = "lat";
    roles[LonRole] = "lon";
    roles[SpeedRole] = "speed";
    roles[TransmissionRadiusRole] = "transmissionRadius";
    return roles;
}

void VehicleModel::setVehicles(const std::vector<Vehicle*>& vehicles)
{
    beginResetModel();
    m_vehicles = vehicles;
    endResetModel();
    emit countChanged();
}

void VehicleModel::updatePositions()
{
    if (!m_vehicles.empty()) {
        emit dataChanged(index(0), index(static_cast<int>(m_vehicles.size()) - 1),
                         {LatRole, LonRole, SpeedRole, TransmissionRadiusRole});
    }
}
