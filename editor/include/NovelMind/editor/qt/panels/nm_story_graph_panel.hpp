#pragma once

/**
 * @file nm_story_graph_panel.hpp
 * @brief Story Graph panel for node-based visual scripting
 *
 * Displays the story graph with:
 * - Node representation
 * - Connection lines
 * - Mini-map
 * - Viewport controls
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>

namespace NovelMind::editor::qt {

/**
 * @brief Graphics item representing a story graph node
 */
class NMGraphNodeItem : public QGraphicsItem
{
public:
    explicit NMGraphNodeItem(const QString& title, const QString& nodeType);

    void setTitle(const QString& title);
    [[nodiscard]] QString title() const { return m_title; }

    void setNodeType(const QString& type);
    [[nodiscard]] QString nodeType() const { return m_nodeType; }

    void setNodeId(uint64_t id) { m_nodeId = id; }
    [[nodiscard]] uint64_t nodeId() const { return m_nodeId; }

    void setSelected(bool selected);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    QString m_title;
    QString m_nodeType;
    uint64_t m_nodeId = 0;
    bool m_isSelected = false;

    static constexpr qreal NODE_WIDTH = 200;
    static constexpr qreal NODE_HEIGHT = 80;
    static constexpr qreal CORNER_RADIUS = 8;
};

/**
 * @brief Graphics item representing a connection between nodes
 */
class NMGraphConnectionItem : public QGraphicsItem
{
public:
    NMGraphConnectionItem(NMGraphNodeItem* startNode, NMGraphNodeItem* endNode);

    void updatePath();

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    NMGraphNodeItem* m_startNode;
    NMGraphNodeItem* m_endNode;
    QPainterPath m_path;
};

/**
 * @brief Graphics scene for the story graph
 */
class NMStoryGraphScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit NMStoryGraphScene(QObject* parent = nullptr);

    /**
     * @brief Add a node to the graph
     */
    NMGraphNodeItem* addNode(const QString& title, const QString& nodeType, const QPointF& pos);

    /**
     * @brief Add a connection between nodes
     */
    NMGraphConnectionItem* addConnection(NMGraphNodeItem* from, NMGraphNodeItem* to);

    /**
     * @brief Clear all nodes and connections
     */
    void clearGraph();

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    QList<NMGraphNodeItem*> m_nodes;
    QList<NMGraphConnectionItem*> m_connections;
};

/**
 * @brief Graphics view for story graph with pan/zoom
 */
class NMStoryGraphView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit NMStoryGraphView(QWidget* parent = nullptr);

    void setZoomLevel(qreal zoom);
    [[nodiscard]] qreal zoomLevel() const { return m_zoomLevel; }

    void centerOnGraph();

signals:
    void zoomChanged(qreal newZoom);
    void nodeClicked(NMGraphNodeItem* node);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    qreal m_zoomLevel = 1.0;
    bool m_isPanning = false;
    QPoint m_lastPanPoint;
};

/**
 * @brief Story Graph panel for visual scripting
 */
class NMStoryGraphPanel : public NMDockPanel
{
    Q_OBJECT

public:
    explicit NMStoryGraphPanel(QWidget* parent = nullptr);
    ~NMStoryGraphPanel() override;

    void onInitialize() override;
    void onUpdate(double deltaTime) override;

    [[nodiscard]] NMStoryGraphScene* graphScene() const { return m_scene; }
    [[nodiscard]] NMStoryGraphView* graphView() const { return m_view; }

    /**
     * @brief Load demo data for testing
     */
    void loadDemoGraph();

signals:
    void nodeSelected(uint64_t nodeId);
    void nodeDoubleClicked(uint64_t nodeId);

private slots:
    void onZoomIn();
    void onZoomOut();
    void onZoomReset();
    void onFitToGraph();

private:
    void setupToolBar();
    void setupContent();

    NMStoryGraphScene* m_scene = nullptr;
    NMStoryGraphView* m_view = nullptr;
    QWidget* m_contentWidget = nullptr;
    QToolBar* m_toolBar = nullptr;
};

} // namespace NovelMind::editor::qt
