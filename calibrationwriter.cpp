#include "calibrationwriter.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <cmath>

bool CalibrationWriter::save(const QUrl& destination, int width, int height, const QVariantList& points)
{
    auto fail = [&](const QString& message) { m_error = message; emit errorChanged(); return false; };
    if (!destination.isLocalFile() || width <= 0 || height <= 0 || points.size() < 4)
        return fail("Choisir un fichier local, une image et au moins quatre correspondances.");
    QJsonArray references;
    for (const auto& entry : points) {
        const auto point = entry.toMap();
        const double x = point.value("x").toDouble(), y = point.value("y").toDouble();
        const double lat = point.value("lat").toDouble(), lon = point.value("lon").toDouble();
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(lat) || !std::isfinite(lon) ||
            x < 0 || y < 0 || x >= width || y >= height || std::abs(lat) > 85 || std::abs(lon) > 180)
            return fail("Un point est hors de l'image ou de la carte.");
        references.append(QJsonObject{{"pixel", QJsonArray{x,y}}, {"geo", QJsonArray{lat,lon}}});
    }
    const auto bytes = QJsonDocument(QJsonObject{
        {"schema_version", 1}, {"image_width", width}, {"image_height", height},
        {"reference_points", references}, {"max_reprojection_error_m", 2.0},
        {"calibration_status", "user_control_points"}
    }).toJson();
    QSaveFile file(destination.toLocalFile());
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        return fail("Impossible d'enregistrer la calibration.");
    m_error.clear(); emit errorChanged();
    return true;
}
