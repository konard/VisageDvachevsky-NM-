#pragma once

/**
 * @file scene_view_panel.hpp
 * @brief Scene View Panel for NovelMind Editor v0.2.0
 *
 * The Scene View panel provides:
 * - WYSIWYG preview of the visual novel scene
 * - Object selection and manipulation via gizmos
 * - Drag-and-drop character/background placement
 * - Layer visibility controls
 * - Transform tools (move, rotate, scale)
 * - Grid and snapping options
 * - Scene preview playback controls
 */

#include "NovelMind/editor/gui_panel_base.hpp"
#include "NovelMind/renderer/color.hpp"
#include <vector>
#include <memory>

namespace NovelMind::editor {

/**
 * @brief Transform tool modes
 */
enum class TransformTool : u8 {
    Select,
    Move,
    Rotate,
    Scale,
    Rect
};

/**
 * @brief Gizmo space (local vs world)
 */
enum class GizmoSpace : u8 {
    Local,
    World
};

/**
 * @brief Scene view render mode
 */
enum class SceneRenderMode : u8 {
    Textured,
    Wireframe,
    Bounds,
    Layers
};

/**
 * @brief Scene View Panel implementation
 */
class SceneViewPanel : public GUIPanelBase {
public:
    SceneViewPanel();
    ~SceneViewPanel() override = default;

    // =========================================================================
    // Tool State
    // =========================================================================

    /**
     * @brief Get current transform tool
     */
    [[nodiscard]] TransformTool getCurrentTool() const { return m_currentTool; }

    /**
     * @brief Set current transform tool
     */
    void setCurrentTool(TransformTool tool);

    /**
     * @brief Get gizmo space
     */
    [[nodiscard]] GizmoSpace getGizmoSpace() const { return m_gizmoSpace; }

    /**
     * @brief Set gizmo space
     */
    void setGizmoSpace(GizmoSpace space) { m_gizmoSpace = space; }

    /**
     * @brief Toggle gizmo space
     */
    void toggleGizmoSpace();

    // =========================================================================
    // View State
    // =========================================================================

    /**
     * @brief Get current zoom level
     */
    [[nodiscard]] f32 getZoom() const { return m_zoom; }

    /**
     * @brief Set zoom level
     */
    void setZoom(f32 zoom);

    /**
     * @brief Zoom to fit selection
     */
    void zoomToFit();

    /**
     * @brief Reset view to default
     */
    void resetView();

    /**
     * @brief Get camera pan offset
     */
    [[nodiscard]] std::pair<f32, f32> getPanOffset() const {
        return {m_panX, m_panY};
    }

    /**
     * @brief Set camera pan offset
     */
    void setPanOffset(f32 x, f32 y);

    // =========================================================================
    // Grid and Snapping
    // =========================================================================

    /**
     * @brief Check if grid is visible
     */
    [[nodiscard]] bool isGridVisible() const { return m_showGrid; }

    /**
     * @brief Set grid visibility
     */
    void setGridVisible(bool visible) { m_showGrid = visible; }

    /**
     * @brief Check if snapping is enabled
     */
    [[nodiscard]] bool isSnappingEnabled() const { return m_snapEnabled; }

    /**
     * @brief Set snapping enabled
     */
    void setSnappingEnabled(bool enabled) { m_snapEnabled = enabled; }

    /**
     * @brief Get snap increment
     */
    [[nodiscard]] f32 getSnapIncrement() const { return m_snapIncrement; }

    /**
     * @brief Set snap increment
     */
    void setSnapIncrement(f32 increment);

    // =========================================================================
    // Layer Controls
    // =========================================================================

    /**
     * @brief Get visible layers mask
     */
    [[nodiscard]] u32 getVisibleLayers() const { return m_visibleLayers; }

    /**
     * @brief Set layer visibility
     */
    void setLayerVisible(u32 layer, bool visible);

    /**
     * @brief Check if layer is visible
     */
    [[nodiscard]] bool isLayerVisible(u32 layer) const;

    /**
     * @brief Show all layers
     */
    void showAllLayers();

    /**
     * @brief Solo a layer (hide all others)
     */
    void soloLayer(u32 layer);

    // =========================================================================
    // Render Mode
    // =========================================================================

    /**
     * @brief Get current render mode
     */
    [[nodiscard]] SceneRenderMode getRenderMode() const { return m_renderMode; }

    /**
     * @brief Set render mode
     */
    void setRenderMode(SceneRenderMode mode) { m_renderMode = mode; }

    // =========================================================================
    // GUIPanelBase Overrides
    // =========================================================================

    [[nodiscard]] std::vector<MenuItem> getMenuItems() const override;
    [[nodiscard]] std::vector<ToolbarItem> getToolbarItems() const override;
    [[nodiscard]] std::vector<MenuItem> getContextMenuItems() const override;

protected:
    void onInitialize() override;
    void onShutdown() override;
    void onUpdate(f64 deltaTime) override;
    void onRender() override;
    void onResize(f32 width, f32 height) override;
    void renderToolbar() override;

    void onSelectionChanged(SelectionType type,
                           const std::vector<SelectionItem>& selection) override;

private:
    // Rendering methods
    void renderSceneContent();
    void renderGrid();
    void renderGizmos();
    void renderPlaceholderObjects();
    void renderSelectionHighlight();
    void renderOverlays();
    void renderLayerControls();

    // Input handling
    void handleMouseInput();
    void handleKeyboardInput();
    void handleDragDrop();

    // Scene interaction
    void selectObjectAtPosition(f32 x, f32 y, bool addToSelection);
    void moveSelectedObjects(f32 deltaX, f32 deltaY);
    void rotateSelectedObjects(f32 angle);
    void scaleSelectedObjects(f32 scaleX, f32 scaleY);

    // Coordinate conversion
    std::pair<f32, f32> screenToScene(f32 screenX, f32 screenY) const;
    std::pair<f32, f32> sceneToScreen(f32 sceneX, f32 sceneY) const;
    f32 snapValue(f32 value) const;

    // Tool state
    TransformTool m_currentTool = TransformTool::Select;
    GizmoSpace m_gizmoSpace = GizmoSpace::World;

    // View state
    f32 m_zoom = 1.0f;
    f32 m_panX = 0.0f;
    f32 m_panY = 0.0f;
    f32 m_minZoom = 0.1f;
    f32 m_maxZoom = 10.0f;

    // Grid and snapping
    bool m_showGrid = true;
    bool m_snapEnabled = false;
    f32 m_snapIncrement = 10.0f;
    f32 m_gridSize = 50.0f;

    // Layer visibility
    u32 m_visibleLayers = 0xFFFFFFFF; // All layers visible

    // Render mode
    SceneRenderMode m_renderMode = SceneRenderMode::Textured;

    // Interaction state
    bool m_isPanning = false;
    bool m_isDragging = false;
    f32 m_dragStartX = 0.0f;
    f32 m_dragStartY = 0.0f;
    f32 m_lastMouseX = 0.0f;
    f32 m_lastMouseY = 0.0f;

    // Scene bounds (for game preview area)
    f32 m_sceneWidth = 1920.0f;
    f32 m_sceneHeight = 1080.0f;

    // Gizmo state
    i32 m_activeGizmoAxis = -1; // -1 = none, 0 = X, 1 = Y, 2 = XY

    // Colors
    renderer::Color m_gridColor{60, 60, 60, 128};
    renderer::Color m_gridMajorColor{80, 80, 80, 200};
    renderer::Color m_selectionColor{0, 122, 204, 255};
    renderer::Color m_gizmoXColor{255, 80, 80, 255};
    renderer::Color m_gizmoYColor{80, 255, 80, 255};
    renderer::Color m_gizmoXYColor{255, 255, 80, 255};
};

} // namespace NovelMind::editor
