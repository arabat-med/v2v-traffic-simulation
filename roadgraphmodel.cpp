#include "roadgraphmodel.h"
#include <QVariantMap>

RoadGraphModel::RoadGraphModel(RoadGraph* graph, QObject *parent)
    : QObject(parent), m_graph(graph)
{
}

void RoadGraphModel::setMaxEdges(int max)
{
    if (m_maxEdges != max) {
        m_maxEdges = max;
        m_edgesCached = false;
        emit maxEdgesChanged();
        emit dataChanged();
    }
}

QVariantList RoadGraphModel::edges() const
{
    if (m_edgesCached) {
        return m_cachedEdges;
    }

    m_cachedEdges.clear();
    if (!m_graph) return m_cachedEdges;

    for (const auto* edge : m_graph->getEdges()) {
        if (edge && edge->from && edge->to) {
            QVariantMap edgeMap;
            edgeMap["fromLat"] = edge->from->lat;
            edgeMap["fromLon"] = edge->from->lon;
            edgeMap["toLat"] = edge->to->lat;
            edgeMap["toLon"] = edge->to->lon;
            edgeMap["roadType"] = QString::fromStdString(edge->road_type);
            m_cachedEdges.append(edgeMap);
        }
    }
    m_edgesCached = true;
    return m_cachedEdges;
}

QVariantList RoadGraphModel::nodes() const
{
    if (m_nodesCached) {
        return m_cachedNodes;
    }

    m_cachedNodes.clear();
    if (!m_graph) return m_cachedNodes;

    for (const auto& pair : m_graph->getNodes()) {
        if (pair.second) {
            QVariantMap nodeMap;
            nodeMap["lat"] = pair.second->lat;
            nodeMap["lon"] = pair.second->lon;
            nodeMap["id"] = (qint64)pair.second->id;
            m_cachedNodes.append(nodeMap);
        }
    }
    m_nodesCached = true;
    return m_cachedNodes;
}

int RoadGraphModel::nodeCount() const
{
    return m_graph ? m_graph->getNodes().size() : 0;
}

int RoadGraphModel::edgeCount() const
{
    return m_graph ? m_graph->getEdges().size() : 0;
}
