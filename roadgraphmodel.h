#ifndef ROADGRAPHMODEL_H
#define ROADGRAPHMODEL_H

#include <QObject>
#include <QVariantList>
#include "roadgraph.h"

class RoadGraphModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList edges READ edges NOTIFY dataChanged)
    Q_PROPERTY(QVariantList nodes READ nodes NOTIFY dataChanged)
    Q_PROPERTY(int nodeCount READ nodeCount NOTIFY dataChanged)
    Q_PROPERTY(int edgeCount READ edgeCount NOTIFY dataChanged)
    Q_PROPERTY(int maxEdges READ maxEdges WRITE setMaxEdges NOTIFY maxEdgesChanged)

public:
    explicit RoadGraphModel(RoadGraph* graph, QObject *parent = nullptr);

    QVariantList edges() const;
    QVariantList nodes() const;
    int nodeCount() const;
    int edgeCount() const;
    int maxEdges() const { return m_maxEdges; }
    void setMaxEdges(int max);

signals:
    void dataChanged();
    void maxEdgesChanged();

private:
    RoadGraph* m_graph;
    mutable QVariantList m_cachedEdges;
    mutable QVariantList m_cachedNodes;
    mutable bool m_edgesCached = false;
    mutable bool m_nodesCached = false;
    int m_maxEdges = 1000000;
};

#endif
