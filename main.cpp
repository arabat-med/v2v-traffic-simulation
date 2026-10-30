#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTimer>
#include <algorithm>
#include <iostream>
#include <sstream>
#include "roadgraph.h"
#include "roadgraphmodel.h"
#include "trafficsimulation.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName("V2V Traffic Simulation");
    QCoreApplication::setApplicationVersion("0.1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("OSM vehicle mobility and geometric V2V range graph");
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption mapOption({"m", "map"}, "Load an external OSM file instead of the embedded demo.", "file");
    const QCommandLineOption vehiclesOption("vehicles", "Initial vehicle count (0 to 2000).", "count");
    const QCommandLineOption startOption("autostart", "Start the simulation after opening the window.");
    const QCommandLineOption smokeOption("smoke-test", "Start, check the QML window and graph, then exit.");
    const QCommandLineOption captureOption("screenshot", "Save a real PNG of the Qt window after 8 seconds, then exit.", "file");
    parser.addOptions({mapOption, vehiclesOption, startOption, smokeOption, captureOption});
    parser.process(app);

    RoadGraph graph;
    QString mapSource;
    bool loaded = false;
    if (parser.isSet(mapOption)) {
        mapSource = QFileInfo(parser.value(mapOption)).absoluteFilePath();
        loaded = graph.loadFromOSM(mapSource.toStdString());
    } else {
        QFile demo(":/data/demo.osm");
        if (demo.open(QIODevice::ReadOnly)) {
            std::istringstream input(demo.readAll().toStdString());
            loaded = graph.loadFromOSM(input);
        }
        mapSource = "embedded data/demo.osm (OpenStreetMap, Bourtzwiller)";
    }
    if (!loaded) {
        std::cerr << "Cannot load a usable road graph: " << mapSource.toStdString() << std::endl;
        return 2;
    }
    std::cout << "Map loaded: " << mapSource.toStdString() << std::endl;
    std::cout << "Road graph: " << graph.getNodes().size() << " nodes, " << graph.getEdges().size() << " directed edges" << std::endl;

    double minLat = 90.0, maxLat = -90.0, minLon = 180.0, maxLon = -180.0;
    for (const auto& pair : graph.getNodes()) {
        minLat = std::min(minLat, pair.second->lat);
        maxLat = std::max(maxLat, pair.second->lat);
        minLon = std::min(minLon, pair.second->lon);
        maxLon = std::max(maxLon, pair.second->lon);
    }

    RoadGraphModel roadGraphModel(&graph);
    TrafficSimulation simulation(&graph);
    if (parser.isSet(vehiclesOption)) {
        bool valid = false;
        const int count = parser.value(vehiclesOption).toInt(&valid);
        if (!valid || count < 0 || count > 2000) {
            std::cerr << "--vehicles must be an integer from 0 to 2000" << std::endl;
            return 2;
        }
        simulation.setVehicleCount(count);
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("mapCenterLat", (minLat + maxLat) / 2.0);
    engine.rootContext()->setContextProperty("mapCenterLon", (minLon + maxLon) / 2.0);
    engine.rootContext()->setContextProperty("mapZoomLevel", parser.isSet(mapOption) ? 13.0 : 15.0);
    engine.rootContext()->setContextProperty("roadGraphModel", &roadGraphModel);
    engine.rootContext()->setContextProperty("trafficSimulation", &simulation);
    engine.rootContext()->setContextProperty("windowTitle", "V2V Traffic Simulation");
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
                     [] { QCoreApplication::exit(3); }, Qt::QueuedConnection);
    engine.loadFromModule("finalproduct", "Main");
    if (engine.rootObjects().isEmpty()) return 3;

    const bool capture = parser.isSet(captureOption);
    if (parser.isSet(startOption) || parser.isSet(smokeOption) || capture) simulation.start();
    if (parser.isSet(smokeOption) || capture) {
        QTimer::singleShot(capture ? 8000 : 750, &app, [&] {
            auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
            if (!window || !window->isVisible() || simulation.interferenceGraph()->nodeCount() != simulation.vehicleCount()) {
                std::cerr << "Application smoke check failed" << std::endl;
                app.exit(4);
                return;
            }
            if (capture) {
                const QString path = QFileInfo(parser.value(captureOption)).absoluteFilePath();
                QDir().mkpath(QFileInfo(path).absolutePath());
                const QImage frame = window->grabWindow();
                if (frame.isNull() || !frame.save(path, "PNG")) {
                    std::cerr << "Cannot save screenshot" << std::endl;
                    app.exit(4);
                    return;
                }
                std::cout << "Screenshot saved: " << path.toStdString() << std::endl;
            }
            std::cout << "SMOKE_TEST_OK: window, road graph and vehicle graph initialized" << std::endl;
            app.exit(0);
        });
    }
    return app.exec();
}
