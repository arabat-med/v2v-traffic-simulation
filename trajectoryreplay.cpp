#include "trajectoryreplay.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <algorithm>
#include <cmath>
#include <limits>

namespace {
bool number(const QJsonObject& object, const char* key, double& value)
{
    const auto entry = object.value(QLatin1String(key));
    if (!entry.isDouble()) return false;
    value = entry.toDouble();
    return std::isfinite(value);
}
}

bool TrajectoryReplay::load(const QString& path, QString& error)
{
    QFile file(path);
    auto fail = [&](const QString& message) { error = message; return false; };
    if (QFileInfo(path).fileName().contains(".partial.")) return fail("Le traitement vidéo est incomplet : importer uniquement trajectories.jsonl finalisé.");
    if (!file.open(QIODevice::ReadOnly)) return fail("Impossible d'ouvrir les trajectoires.");
    if (file.size() > 64 * 1024 * 1024) return fail("Fichier trop volumineux (maximum 64 Mio).");
    TrajectoryReplay parsed;
    QJsonParseError jsonError;
    auto document = QJsonDocument::fromJson(file.readLine(), &jsonError);
    const auto meta = document.object();
    if (jsonError.error != QJsonParseError::NoError ||
        meta.value("type") != "v2v_trajectories" || meta.value("schema_version").toInt() != 1 ||
        meta.value("coordinate_system") != "WGS84" || meta.value("position_source") != "camera_homography")
        return fail("Format invalide : utiliser trajectories.jsonl issu d'une vidéo calibrée.");
    const auto status = meta.value("calibration_status").toString();
    if (status != "user_control_points" && status != "synthetic_test")
        return fail("Le statut de calibration est absent ou inconnu.");
    parsed.synthetic = status == "synthetic_test";
    if (!number(meta, "transmission_radius_m", parsed.radius) || parsed.radius <= 0 || parsed.radius > 5000)
        return fail("Rayon de portée simulée invalide.");
    double minLat = 90, maxLat = -90, minLon = 180, maxLon = -180;
    bool hasVehicles = false;
    int line = 1;
    while (!file.atEnd()) {
        ++line;
        const auto bytes = file.readLine();
        if (bytes.trimmed().isEmpty()) continue;
        document = QJsonDocument::fromJson(bytes, &jsonError);
        const auto object = document.object();
        ObservedFrame frame;
        if (jsonError.error != QJsonParseError::NoError || !document.isObject() ||
            !number(object, "time_s", frame.time) || frame.time < 0 ||
            (!parsed.frames.empty() && frame.time <= parsed.frames.back().time) ||
            !object.value("vehicles").isArray())
            return fail(QString("Image invalide ou temps non croissant, ligne %1.").arg(line));
        const auto vehicles = object.value("vehicles").toArray();
        if (vehicles.size() > 2000 || parsed.frames.size() >= 100000)
            return fail("Limite dépassée : 2 000 véhicules par image ou 100 000 images.");
        QSet<qint64> ids;
        for (const auto& entry : vehicles) {
            const auto vehicle = entry.toObject();
            double id;
            ObservedVehicle observation;
            if (!number(vehicle, "id", id) || id < 1 || id > 9007199254740991.0 || std::floor(id) != id ||
                !number(vehicle, "lat", observation.lat) || std::abs(observation.lat) > 85 ||
                !number(vehicle, "lon", observation.lon) || std::abs(observation.lon) > 180 ||
                !number(vehicle, "speed_mps", observation.speed) || observation.speed < 0)
                return fail(QString("Coordonnées, identifiant ou vitesse invalides, ligne %1.").arg(line));
            observation.id = static_cast<long long>(id);
            if (ids.contains(observation.id)) return fail("Identifiant présent deux fois dans une image.");
            ids.insert(observation.id);
            frame.vehicles.push_back(observation);
            minLat = std::min(minLat, observation.lat); maxLat = std::max(maxLat, observation.lat);
            minLon = std::min(minLon, observation.lon); maxLon = std::max(maxLon, observation.lon);
            hasVehicles = true;
        }
        parsed.frames.push_back(std::move(frame));
    }
    if (parsed.frames.empty() || !hasVehicles) return fail("Aucun véhicule géolocalisé dans ce fichier.");
    if (parsed.frames.front().time != 0.0) return fail("La première image doit être à time_s = 0.");
    parsed.centerLat = (minLat + maxLat) / 2;
    parsed.centerLon = (minLon + maxLon) / 2;
    *this = std::move(parsed);
    error.clear();
    return true;
}

int TrajectoryReplay::frameAt(double time) const
{
    const auto found = std::upper_bound(frames.begin(), frames.end(), time,
        [](double t, const ObservedFrame& frame) { return t < frame.time; });
    return found == frames.begin() ? 0 : static_cast<int>(found - frames.begin() - 1);
}
