#include "NovelMind/editor/qt/panels/nm_story_graph_panel.hpp"
#include "NovelMind/editor/qt/nm_style_manager.hpp"

#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <QGraphicsSceneMouseEvent>

namespace NovelMind::editor::qt {

// ============================================================================
// NMGraphNodeItem
// ============================================================================

NMGraphNodeItem::NMGraphNodeItem(const QString& title, const QString& nodeType)
    : m_title(title)
    , m_nodeType(nodeType)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
}

void NMGraphNodeItem::setTitle(const QString& title)
{
    m_title = title;
    update();
}

void NMGraphNodeItem::setNodeType(const QString& type)
{
    m_nodeType = type;
    update();
}

void NMGraphNodeItem::setSelected(bool selected)
{
    m_isSelected = selected;
    QGraphicsItem::setSelected(selected);
    update();
}

QRectF NMGraphNodeItem::boundingRect() const
{
    return QRectF(0, 0, NODE_WIDTH, NODE_HEIGHT);
}

void NMGraphNodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    const auto& palette = NMStyleManager::instance().palette();

    painter->setRenderHint(QPainter::Antialiasing);

    // Node background
    QColor bgColor = m_isSelected ? palette.nodeSelected : palette.nodeDefault;
    painter->setBrush(bgColor);
    painter->setPen(QPen(palette.borderLight, 1));
    painter->drawRoundedRect(boundingRect(), CORNER_RADIUS, CORNER_RADIUS);

    // Header bar
    QRectF headerRect(0, 0, NODE_WIDTH, 24);
    painter->setBrush(palette.bgDark);
    painter->setPen(Qt::NoPen);
    QPainterPath headerPath;
    headerPath.addRoundedRect(headerRect, CORNER_RADIUS, CORNER_RADIUS);
    // Clip to top corners only
    QPainterPath clipPath;
    clipPath.addRect(QRectF(0, CORNER_RADIUS, NODE_WIDTH, 24 - CORNER_RADIUS));
    headerPath = headerPath.united(clipPath);
    painter->drawPath(headerPath);

    // Node type (header)
    painter->setPen(palette.textSecondary);
    painter->setFont(NMStyleManager::instance().defaultFont());
    painter->drawText(headerRect.adjusted(8, 0, -8, 0), Qt::AlignVCenter | Qt::AlignLeft, m_nodeType);

    // Node title (body)
    QRectF titleRect(8, 30, NODE_WIDTH - 16, NODE_HEIGHT - 38);
    painter->setPen(palette.textPrimary);
    QFont boldFont = NMStyleManager::instance().defaultFont();
    boldFont.setBold(true);
    painter->setFont(boldFont);
    painter->drawText(titleRect, Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, m_title);

    // Selection highlight
    if (m_isSelected)
    {
        painter->setPen(QPen(palette.accentPrimary, 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(boundingRect().adjusted(1, 1, -1, -1), CORNER_RADIUS, CORNER_RADIUS);
    }
}

QVariant NMGraphNodeItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged)
    {
        // Update connections when node moves
        // This would be handled by the parent scene
    }
    else if (change == ItemSelectedHasChanged)
    {
        m_isSelected = value.toBool();
    }
    return QGraphicsItem::itemChange(change, value);
}

// ============================================================================
// NMGraphConnectionItem
// ============================================================================

NMGraphConnectionItem::NMGraphConnectionItem(NMGraphNodeItem* startNode, NMGraphNodeItem* endNode)
    : m_startNode(startNode)
    , m_endNode(endNode)
{
    setZValue(-1); // Draw behind nodes
    updatePath();
}

void NMGraphConnectionItem::updatePath()
{
    if (!m_startNode || !m_endNode) return;

    QPointF start = m_startNode->sceneBoundingRect().center();
    start.setX(m_startNode->sceneBoundingRect().right());

    QPointF end = m_endNode->sceneBoundingRect().center();
    end.setX(m_endNode->sceneBoundingRect().left());

    // Create bezier curve
    m_path = QPainterPath();
    m_path.moveTo(start);

    qreal dx = std::abs(end.x() - start.x()) * 0.5;
    m_path.cubicTo(start + QPointF(dx, 0),
                   end + QPointF(-dx, 0),
                   end);

    prepareGeometryChange();
}

QRectF NMGraphConnectionItem::boundingRect() const
{
    return m_path.boundingRect().adjusted(-5, -5, 5, 5);
}

void NMGraphConnectionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    const auto& palette = NMStyleManager::instance().palette();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(palette.connectionLine, 2));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(m_path);
}

// ============================================================================
// NMStoryGraphScene
// ============================================================================

NMStoryGraphScene::NMStoryGraphScene(QObject* parent)
    : QGraphicsScene(parent)
{
    setSceneRect(-5000, -5000, 10000, 10000);
}

NMGraphNodeItem* NMStoryGraphScene::addNode(const QString& title, const QString& nodeType, const QPointF& pos)
{
    auto* node = new NMGraphNodeItem(title, nodeType);
    node->setPos(pos);
    addItem(node);
    m_nodes.append(node);
    return node;
}

NMGraphConnectionItem* NMStoryGraphScene::addConnection(NMGraphNodeItem* from, NMGraphNodeItem* to)
{
    auto* connection = new NMGraphConnectionItem(from, to);
    addItem(connection);
    m_connections.append(connection);
    return connection;
}

void NMStoryGraphScene::clearGraph()
{
    for (auto* conn : m_connections)
    {
        removeItem(conn);
        delete conn;
    }
    m_connections.clear();

    for (auto* node : m_nodes)
    {
        removeItem(node);
        delete node;
    }
    m_nodes.clear();
}

void NMStoryGraphScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    const auto& palette = NMStyleManager::instance().palette();

    // Fill background
    painter->fillRect(rect, palette.bgDarkest);

    // Draw grid (dots pattern for graph view)
    painter->setPen(palette.gridLine);

    qreal gridSize = 32.0;
    qreal left = rect.left() - std::fmod(rect.left(), gridSize);
    qreal top = rect.top() - std::fmod(rect.top(), gridSize);

    for (qreal x = left; x < rect.right(); x += gridSize)
    {
        for (qreal y = top; y < rect.bottom(); y += gridSize)
        {
            painter->drawPoint(QPointF(x, y));
        }
    }

    // Draw origin
    painter->setPen(QPen(palette.accentPrimary, 1));
    if (rect.left() <= 0 && rect.right() >= 0)
    {
        painter->drawLine(QLineF(0, rect.top(), 0, rect.bottom()));
    }
    if (rect.top() <= 0 && rect.bottom() >= 0)
    {
        painter->drawLine(QLineF(rect.left(), 0, rect.right(), 0));
    }
}

// ============================================================================
// NMStoryGraphView
// ============================================================================

NMStoryGraphView::NMStoryGraphView(QWidget* parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(AnchorUnderMouse);
    setResizeAnchor(AnchorViewCenter);
    setDragMode(RubberBandDrag);
}

void NMStoryGraphView::setZoomLevel(qreal zoom)
{
    zoom = qBound(0.1, zoom, 5.0);
    if (qFuzzyCompare(m_zoomLevel, zoom)) return;

    qreal scaleFactor = zoom / m_zoomLevel;
    m_zoomLevel = zoom;

    scale(scaleFactor, scaleFactor);
    emit zoomChanged(m_zoomLevel);
}

void NMStoryGraphView::centerOnGraph()
{
    if (scene() && !scene()->items().isEmpty())
    {
        centerOn(scene()->itemsBoundingRect().center());
    }
    else
    {
        centerOn(0, 0);
    }
}

void NMStoryGraphView::wheelEvent(QWheelEvent* event)
{
    qreal factor = 1.15;
    if (event->angleDelta().y() < 0)
    {
        factor = 1.0 / factor;
    }

    setZoomLevel(m_zoomLevel * factor);
    event->accept();
}

void NMStoryGraphView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        m_isPanning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void NMStoryGraphView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isPanning)
    {
        QPoint delta = event->pos() - m_lastPanPoint;
        m_lastPanPoint = event->pos();

        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void NMStoryGraphView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton && m_isPanning)
    {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

// ============================================================================
// NMStoryGraphPanel
// ============================================================================

NMStoryGraphPanel::NMStoryGraphPanel(QWidget* parent)
    : NMDockPanel(tr("Story Graph"), parent)
{
    setPanelId("StoryGraph");
    setupContent();
    setupToolBar();
}

NMStoryGraphPanel::~NMStoryGraphPanel() = default;

void NMStoryGraphPanel::onInitialize()
{
    // Load demo data for testing
    loadDemoGraph();

    if (m_view)
    {
        m_view->centerOnGraph();
    }
}

void NMStoryGraphPanel::onUpdate(double /*deltaTime*/)
{
    // Update connections when nodes move
    // For now, this is handled reactively
}

void NMStoryGraphPanel::loadDemoGraph()
{
    if (!m_scene) return;

    m_scene->clearGraph();

    // Create demo nodes
    auto* startNode = m_scene->addNode("Start", "Entry", QPointF(0, 0));
    startNode->setNodeId(1);

    auto* dialogueNode = m_scene->addNode("Welcome to the game!", "Dialogue", QPointF(300, 0));
    dialogueNode->setNodeId(2);

    auto* choiceNode = m_scene->addNode("What will you do?", "Choice", QPointF(600, 0));
    choiceNode->setNodeId(3);

    auto* branch1 = m_scene->addNode("You explore the forest", "Dialogue", QPointF(900, -100));
    branch1->setNodeId(4);

    auto* branch2 = m_scene->addNode("You head to the village", "Dialogue", QPointF(900, 100));
    branch2->setNodeId(5);

    // Create connections
    m_scene->addConnection(startNode, dialogueNode);
    m_scene->addConnection(dialogueNode, choiceNode);
    m_scene->addConnection(choiceNode, branch1);
    m_scene->addConnection(choiceNode, branch2);
}

void NMStoryGraphPanel::setupToolBar()
{
    m_toolBar = new QToolBar(this);
    m_toolBar->setObjectName("StoryGraphToolBar");
    m_toolBar->setIconSize(QSize(16, 16));

    QAction* actionZoomIn = m_toolBar->addAction(tr("+"));
    actionZoomIn->setToolTip(tr("Zoom In"));
    connect(actionZoomIn, &QAction::triggered, this, &NMStoryGraphPanel::onZoomIn);

    QAction* actionZoomOut = m_toolBar->addAction(tr("-"));
    actionZoomOut->setToolTip(tr("Zoom Out"));
    connect(actionZoomOut, &QAction::triggered, this, &NMStoryGraphPanel::onZoomOut);

    QAction* actionZoomReset = m_toolBar->addAction(tr("1:1"));
    actionZoomReset->setToolTip(tr("Reset Zoom"));
    connect(actionZoomReset, &QAction::triggered, this, &NMStoryGraphPanel::onZoomReset);

    QAction* actionFit = m_toolBar->addAction(tr("Fit"));
    actionFit->setToolTip(tr("Fit Graph to View"));
    connect(actionFit, &QAction::triggered, this, &NMStoryGraphPanel::onFitToGraph);

    if (auto* layout = qobject_cast<QVBoxLayout*>(m_contentWidget->layout()))
    {
        layout->insertWidget(0, m_toolBar);
    }
}

void NMStoryGraphPanel::setupContent()
{
    m_contentWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(m_contentWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_scene = new NMStoryGraphScene(this);
    m_view = new NMStoryGraphView(m_contentWidget);
    m_view->setScene(m_scene);

    layout->addWidget(m_view);

    setContentWidget(m_contentWidget);
}

void NMStoryGraphPanel::onZoomIn()
{
    if (m_view)
    {
        m_view->setZoomLevel(m_view->zoomLevel() * 1.25);
    }
}

void NMStoryGraphPanel::onZoomOut()
{
    if (m_view)
    {
        m_view->setZoomLevel(m_view->zoomLevel() / 1.25);
    }
}

void NMStoryGraphPanel::onZoomReset()
{
    if (m_view)
    {
        m_view->setZoomLevel(1.0);
        m_view->centerOnGraph();
    }
}

void NMStoryGraphPanel::onFitToGraph()
{
    if (m_view && m_scene && !m_scene->items().isEmpty())
    {
        m_view->fitInView(m_scene->itemsBoundingRect().adjusted(-50, -50, 50, 50), Qt::KeepAspectRatio);
    }
}

} // namespace NovelMind::editor::qt
