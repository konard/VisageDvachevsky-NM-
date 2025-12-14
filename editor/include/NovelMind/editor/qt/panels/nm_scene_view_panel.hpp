#pragma once

/**
 * @file nm_scene_view_panel.hpp
 * @brief Scene View panel for visual scene editing
 *
 * Displays the visual novel scene with:
 * - Background image
 * - Character sprites
 * - UI elements
 * - Selection highlighting
 * - Viewport controls (pan, zoom)
 */

#include "NovelMind/editor/qt/nm_dock_panel.hpp"
#include <QGraphicsView>
#include <QGraphicsScene>

namespace NovelMind::editor::qt {

/**
 * @brief Graphics scene for the scene view
 */
class NMSceneGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit NMSceneGraphicsScene(QObject* parent = nullptr);

    void setGridVisible(bool visible);
    [[nodiscard]] bool isGridVisible() const { return m_gridVisible; }

    void setGridSize(qreal size);
    [[nodiscard]] qreal gridSize() const { return m_gridSize; }

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    bool m_gridVisible = true;
    qreal m_gridSize = 32.0;
};

/**
 * @brief Graphics view with pan and zoom support
 */
class NMSceneGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit NMSceneGraphicsView(QWidget* parent = nullptr);

    void setZoomLevel(qreal zoom);
    [[nodiscard]] qreal zoomLevel() const { return m_zoomLevel; }

    void centerOnScene();
    void fitToScene();

signals:
    void zoomChanged(qreal newZoom);

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
 * @brief Scene View panel for visual scene editing
 */
class NMSceneViewPanel : public NMDockPanel
{
    Q_OBJECT

public:
    explicit NMSceneViewPanel(QWidget* parent = nullptr);
    ~NMSceneViewPanel() override;

    void onInitialize() override;
    void onUpdate(double deltaTime) override;

    /**
     * @brief Get the graphics scene
     */
    [[nodiscard]] NMSceneGraphicsScene* graphicsScene() const { return m_scene; }

    /**
     * @brief Get the graphics view
     */
    [[nodiscard]] NMSceneGraphicsView* graphicsView() const { return m_view; }

    /**
     * @brief Set grid visibility
     */
    void setGridVisible(bool visible);

    /**
     * @brief Set zoom level
     */
    void setZoomLevel(qreal zoom);

signals:
    void objectSelected(const QString& objectId);
    void objectDoubleClicked(const QString& objectId);

private slots:
    void onZoomIn();
    void onZoomOut();
    void onZoomReset();
    void onToggleGrid();

private:
    void setupToolBar();
    void setupContent();

    NMSceneGraphicsScene* m_scene = nullptr;
    NMSceneGraphicsView* m_view = nullptr;
    QWidget* m_contentWidget = nullptr;
    QToolBar* m_toolBar = nullptr;
};

} // namespace NovelMind::editor::qt
