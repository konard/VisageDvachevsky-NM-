#include "NovelMind/editor/qt/panels/nm_scene_view_panel.hpp"
#include "NovelMind/editor/qt/nm_style_manager.hpp"

#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>

namespace NovelMind::editor::qt {

// ============================================================================
// NMSceneGraphicsScene
// ============================================================================

NMSceneGraphicsScene::NMSceneGraphicsScene(QObject* parent)
    : QGraphicsScene(parent)
{
    // Set a large scene rect for scrolling
    setSceneRect(-5000, -5000, 10000, 10000);
}

void NMSceneGraphicsScene::setGridVisible(bool visible)
{
    m_gridVisible = visible;
    invalidate(sceneRect(), BackgroundLayer);
}

void NMSceneGraphicsScene::setGridSize(qreal size)
{
    m_gridSize = size;
    if (m_gridVisible)
    {
        invalidate(sceneRect(), BackgroundLayer);
    }
}

void NMSceneGraphicsScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    const auto& palette = NMStyleManager::instance().palette();

    // Fill background
    painter->fillRect(rect, palette.bgDarkest);

    if (!m_gridVisible) return;

    // Draw grid
    painter->setPen(QPen(palette.gridLine, 1));

    // Calculate grid bounds
    qreal left = rect.left() - std::fmod(rect.left(), m_gridSize);
    qreal top = rect.top() - std::fmod(rect.top(), m_gridSize);

    // Draw minor grid lines
    QVector<QLineF> lines;
    for (qreal x = left; x < rect.right(); x += m_gridSize)
    {
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    }
    for (qreal y = top; y < rect.bottom(); y += m_gridSize)
    {
        lines.append(QLineF(rect.left(), y, rect.right(), y));
    }
    painter->drawLines(lines);

    // Draw major grid lines (every 8 minor lines)
    painter->setPen(QPen(palette.gridMajor, 1));
    qreal majorSize = m_gridSize * 8;
    left = rect.left() - std::fmod(rect.left(), majorSize);
    top = rect.top() - std::fmod(rect.top(), majorSize);

    lines.clear();
    for (qreal x = left; x < rect.right(); x += majorSize)
    {
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    }
    for (qreal y = top; y < rect.bottom(); y += majorSize)
    {
        lines.append(QLineF(rect.left(), y, rect.right(), y));
    }
    painter->drawLines(lines);

    // Draw origin axes
    painter->setPen(QPen(palette.accentPrimary, 2));
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
// NMSceneGraphicsView
// ============================================================================

NMSceneGraphicsView::NMSceneGraphicsView(QWidget* parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(AnchorUnderMouse);
    setResizeAnchor(AnchorViewCenter);
    setDragMode(NoDrag);

    // Set background
    setBackgroundBrush(Qt::NoBrush);
}

void NMSceneGraphicsView::setZoomLevel(qreal zoom)
{
    zoom = qBound(0.1, zoom, 10.0);
    if (qFuzzyCompare(m_zoomLevel, zoom)) return;

    qreal scaleFactor = zoom / m_zoomLevel;
    m_zoomLevel = zoom;

    scale(scaleFactor, scaleFactor);
    emit zoomChanged(m_zoomLevel);
}

void NMSceneGraphicsView::centerOnScene()
{
    centerOn(0, 0);
}

void NMSceneGraphicsView::fitToScene()
{
    if (scene() && !scene()->items().isEmpty())
    {
        fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
        m_zoomLevel = transform().m11();
        emit zoomChanged(m_zoomLevel);
    }
}

void NMSceneGraphicsView::wheelEvent(QWheelEvent* event)
{
    // Zoom with mouse wheel
    qreal factor = 1.15;
    if (event->angleDelta().y() < 0)
    {
        factor = 1.0 / factor;
    }

    setZoomLevel(m_zoomLevel * factor);
    event->accept();
}

void NMSceneGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        // Start panning
        m_isPanning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void NMSceneGraphicsView::mouseMoveEvent(QMouseEvent* event)
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

void NMSceneGraphicsView::mouseReleaseEvent(QMouseEvent* event)
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
// NMSceneViewPanel
// ============================================================================

NMSceneViewPanel::NMSceneViewPanel(QWidget* parent)
    : NMDockPanel(tr("Scene View"), parent)
{
    setPanelId("SceneView");
    setupContent();
    setupToolBar();
}

NMSceneViewPanel::~NMSceneViewPanel() = default;

void NMSceneViewPanel::onInitialize()
{
    // Center the view on origin
    if (m_view)
    {
        m_view->centerOnScene();
    }
}

void NMSceneViewPanel::onUpdate(double /*deltaTime*/)
{
    // Continuous updates for animations, etc.
    // For now, no continuous update needed
}

void NMSceneViewPanel::setGridVisible(bool visible)
{
    if (m_scene)
    {
        m_scene->setGridVisible(visible);
    }
}

void NMSceneViewPanel::setZoomLevel(qreal zoom)
{
    if (m_view)
    {
        m_view->setZoomLevel(zoom);
    }
}

void NMSceneViewPanel::setupToolBar()
{
    m_toolBar = new QToolBar(this);
    m_toolBar->setObjectName("SceneViewToolBar");
    m_toolBar->setIconSize(QSize(16, 16));

    QAction* actionZoomIn = m_toolBar->addAction(tr("+"));
    actionZoomIn->setToolTip(tr("Zoom In"));
    connect(actionZoomIn, &QAction::triggered, this, &NMSceneViewPanel::onZoomIn);

    QAction* actionZoomOut = m_toolBar->addAction(tr("-"));
    actionZoomOut->setToolTip(tr("Zoom Out"));
    connect(actionZoomOut, &QAction::triggered, this, &NMSceneViewPanel::onZoomOut);

    QAction* actionZoomReset = m_toolBar->addAction(tr("1:1"));
    actionZoomReset->setToolTip(tr("Reset Zoom"));
    connect(actionZoomReset, &QAction::triggered, this, &NMSceneViewPanel::onZoomReset);

    m_toolBar->addSeparator();

    QAction* actionToggleGrid = m_toolBar->addAction(tr("Grid"));
    actionToggleGrid->setToolTip(tr("Toggle Grid"));
    actionToggleGrid->setCheckable(true);
    actionToggleGrid->setChecked(true);
    connect(actionToggleGrid, &QAction::toggled, this, &NMSceneViewPanel::onToggleGrid);

    // Insert toolbar at top of content widget
    if (auto* layout = qobject_cast<QVBoxLayout*>(m_contentWidget->layout()))
    {
        layout->insertWidget(0, m_toolBar);
    }
}

void NMSceneViewPanel::setupContent()
{
    m_contentWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(m_contentWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Create graphics scene and view
    m_scene = new NMSceneGraphicsScene(this);
    m_view = new NMSceneGraphicsView(m_contentWidget);
    m_view->setScene(m_scene);

    layout->addWidget(m_view);

    setContentWidget(m_contentWidget);
}

void NMSceneViewPanel::onZoomIn()
{
    if (m_view)
    {
        m_view->setZoomLevel(m_view->zoomLevel() * 1.25);
    }
}

void NMSceneViewPanel::onZoomOut()
{
    if (m_view)
    {
        m_view->setZoomLevel(m_view->zoomLevel() / 1.25);
    }
}

void NMSceneViewPanel::onZoomReset()
{
    if (m_view)
    {
        m_view->setZoomLevel(1.0);
        m_view->centerOnScene();
    }
}

void NMSceneViewPanel::onToggleGrid()
{
    if (m_scene)
    {
        m_scene->setGridVisible(!m_scene->isGridVisible());
    }
}

} // namespace NovelMind::editor::qt
