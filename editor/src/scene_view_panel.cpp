/**
 * @file scene_view_panel.cpp
 * @brief Scene View Panel implementation
 */

#include "NovelMind/editor/scene_view_panel.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include <algorithm>
#include <cmath>

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
#include <imgui.h>
#endif

namespace NovelMind::editor {

SceneViewPanel::SceneViewPanel()
    : GUIPanelBase("Scene View")
{
    m_flags = PanelFlags::MenuBar;
}

void SceneViewPanel::setCurrentTool(TransformTool tool)
{
    if (m_currentTool != tool)
    {
        m_currentTool = tool;
        m_activeGizmoAxis = -1;
    }
}

void SceneViewPanel::toggleGizmoSpace()
{
    m_gizmoSpace = (m_gizmoSpace == GizmoSpace::Local) ?
                   GizmoSpace::World : GizmoSpace::Local;
}

void SceneViewPanel::setZoom(f32 zoom)
{
    m_zoom = std::clamp(zoom, m_minZoom, m_maxZoom);
}

void SceneViewPanel::zoomToFit()
{
    // Calculate zoom to fit scene in viewport
    f32 zoomX = m_contentWidth / m_sceneWidth;
    f32 zoomY = m_contentHeight / m_sceneHeight;
    setZoom(std::min(zoomX, zoomY) * 0.9f);
    m_panX = 0.0f;
    m_panY = 0.0f;
}

void SceneViewPanel::resetView()
{
    m_zoom = 1.0f;
    m_panX = 0.0f;
    m_panY = 0.0f;
}

void SceneViewPanel::setPanOffset(f32 x, f32 y)
{
    m_panX = x;
    m_panY = y;
}

void SceneViewPanel::setSnapIncrement(f32 increment)
{
    m_snapIncrement = std::max(1.0f, increment);
}

void SceneViewPanel::setLayerVisible(u32 layer, bool visible)
{
    if (layer >= 32) return;

    if (visible)
    {
        m_visibleLayers |= (1u << layer);
    }
    else
    {
        m_visibleLayers &= ~(1u << layer);
    }
}

bool SceneViewPanel::isLayerVisible(u32 layer) const
{
    if (layer >= 32) return false;
    return (m_visibleLayers & (1u << layer)) != 0;
}

void SceneViewPanel::showAllLayers()
{
    m_visibleLayers = 0xFFFFFFFF;
}

void SceneViewPanel::soloLayer(u32 layer)
{
    if (layer >= 32) return;
    m_visibleLayers = (1u << layer);
}

std::vector<MenuItem> SceneViewPanel::getMenuItems() const
{
    std::vector<MenuItem> items;

    // View menu items
    MenuItem viewMenu;
    viewMenu.label = "View";
    viewMenu.subItems = {
        {"Reset View", "Home", [this]() { const_cast<SceneViewPanel*>(this)->resetView(); }},
        {"Zoom to Fit", "Shift+F", [this]() { const_cast<SceneViewPanel*>(this)->zoomToFit(); }},
        MenuItem::separator(),
        {"Show Grid", "", [this]() { const_cast<SceneViewPanel*>(this)->setGridVisible(!m_showGrid); },
         []() { return true; }, [this]() { return m_showGrid; }},
        {"Enable Snapping", "", [this]() { const_cast<SceneViewPanel*>(this)->setSnappingEnabled(!m_snapEnabled); },
         []() { return true; }, [this]() { return m_snapEnabled; }},
    };
    items.push_back(viewMenu);

    // Render mode menu
    MenuItem renderMenu;
    renderMenu.label = "Render Mode";
    renderMenu.subItems = {
        {"Textured", "", [this]() { const_cast<SceneViewPanel*>(this)->setRenderMode(SceneRenderMode::Textured); },
         []() { return true; }, [this]() { return m_renderMode == SceneRenderMode::Textured; }},
        {"Wireframe", "", [this]() { const_cast<SceneViewPanel*>(this)->setRenderMode(SceneRenderMode::Wireframe); },
         []() { return true; }, [this]() { return m_renderMode == SceneRenderMode::Wireframe; }},
        {"Bounds", "", [this]() { const_cast<SceneViewPanel*>(this)->setRenderMode(SceneRenderMode::Bounds); },
         []() { return true; }, [this]() { return m_renderMode == SceneRenderMode::Bounds; }},
        {"Layers", "", [this]() { const_cast<SceneViewPanel*>(this)->setRenderMode(SceneRenderMode::Layers); },
         []() { return true; }, [this]() { return m_renderMode == SceneRenderMode::Layers; }},
    };
    items.push_back(renderMenu);

    return items;
}

std::vector<ToolbarItem> SceneViewPanel::getToolbarItems() const
{
    std::vector<ToolbarItem> items;

    // Transform tools
    items.push_back({"Q", "Select (Q)", [this]() { const_cast<SceneViewPanel*>(this)->setCurrentTool(TransformTool::Select); },
                     []() { return true; }, [this]() { return m_currentTool == TransformTool::Select; }});
    items.push_back({"W", "Move (W)", [this]() { const_cast<SceneViewPanel*>(this)->setCurrentTool(TransformTool::Move); },
                     []() { return true; }, [this]() { return m_currentTool == TransformTool::Move; }});
    items.push_back({"E", "Rotate (E)", [this]() { const_cast<SceneViewPanel*>(this)->setCurrentTool(TransformTool::Rotate); },
                     []() { return true; }, [this]() { return m_currentTool == TransformTool::Rotate; }});
    items.push_back({"R", "Scale (R)", [this]() { const_cast<SceneViewPanel*>(this)->setCurrentTool(TransformTool::Scale); },
                     []() { return true; }, [this]() { return m_currentTool == TransformTool::Scale; }});
    items.push_back({"T", "Rect Tool (T)", [this]() { const_cast<SceneViewPanel*>(this)->setCurrentTool(TransformTool::Rect); },
                     []() { return true; }, [this]() { return m_currentTool == TransformTool::Rect; }});

    items.push_back(ToolbarItem::separator());

    // Gizmo space toggle
    items.push_back({"L/G", "Toggle Local/Global Space",
                     [this]() { const_cast<SceneViewPanel*>(this)->toggleGizmoSpace(); },
                     []() { return true; }, [this]() { return m_gizmoSpace == GizmoSpace::Local; }});

    items.push_back(ToolbarItem::separator());

    // View controls
    items.push_back({"#", "Toggle Grid", [this]() { const_cast<SceneViewPanel*>(this)->setGridVisible(!m_showGrid); },
                     []() { return true; }, [this]() { return m_showGrid; }});
    items.push_back({"S", "Toggle Snapping", [this]() { const_cast<SceneViewPanel*>(this)->setSnappingEnabled(!m_snapEnabled); },
                     []() { return true; }, [this]() { return m_snapEnabled; }});

    return items;
}

std::vector<MenuItem> SceneViewPanel::getContextMenuItems() const
{
    std::vector<MenuItem> items;

    bool hasSelection = getSelection().hasSelection();

    items.push_back({"Cut", "Ctrl+X", []() { /* Cut action */ }, [hasSelection]() { return hasSelection; }});
    items.push_back({"Copy", "Ctrl+C", []() { /* Copy action */ }, [hasSelection]() { return hasSelection; }});
    items.push_back({"Paste", "Ctrl+V", []() { /* Paste action */ }});
    items.push_back({"Delete", "Delete", []() { /* Delete action */ }, [hasSelection]() { return hasSelection; }});
    items.push_back(MenuItem::separator());
    items.push_back({"Duplicate", "Ctrl+D", []() { /* Duplicate action */ }, [hasSelection]() { return hasSelection; }});
    items.push_back(MenuItem::separator());
    items.push_back({"Create Empty", "", []() { /* Create empty object */ }});
    items.push_back({"Create Character", "", []() { /* Create character sprite */ }});
    items.push_back({"Create Background", "", []() { /* Create background */ }});
    items.push_back({"Create UI Element", "", []() { /* Create UI element */ }});

    return items;
}

void SceneViewPanel::onInitialize()
{
    // Subscribe to relevant events
    subscribeEvent<SceneObjectTransformedEvent>(
        [this](const SceneObjectTransformedEvent& /*event*/) {
            // Refresh view when objects are transformed externally
        });

    // Reset view when project events occur
    subscribeEvent<ProjectModifiedEvent>(
        [this](const ProjectModifiedEvent& /*event*/) {
            // Optional: reset view when project is modified
        });
}

void SceneViewPanel::onShutdown()
{
    // Cleanup
}

void SceneViewPanel::onUpdate(f64 /*deltaTime*/)
{
    handleMouseInput();
    handleKeyboardInput();
}

void SceneViewPanel::onRender()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Get content region for rendering
    ImVec2 contentMin = ImGui::GetCursorScreenPos();
    ImVec2 contentMax = ImVec2(contentMin.x + m_contentWidth, contentMin.y + m_contentHeight);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw scene background
    drawList->AddRectFilled(contentMin, contentMax, IM_COL32(30, 30, 30, 255));

    // Draw scene bounds indicator (visual novel canvas area)
    f32 sceneLeft, sceneTop, sceneRight, sceneBottom;
    std::tie(sceneLeft, sceneTop) = sceneToScreen(0, 0);
    std::tie(sceneRight, sceneBottom) = sceneToScreen(m_sceneWidth, m_sceneHeight);

    // Offset by content area position
    sceneLeft += contentMin.x;
    sceneTop += contentMin.y;
    sceneRight += contentMin.x;
    sceneBottom += contentMin.y;

    // Draw canvas area
    drawList->AddRectFilled(
        ImVec2(sceneLeft, sceneTop),
        ImVec2(sceneRight, sceneBottom),
        IM_COL32(45, 45, 45, 255));
    drawList->AddRect(
        ImVec2(sceneLeft, sceneTop),
        ImVec2(sceneRight, sceneBottom),
        IM_COL32(100, 100, 100, 255), 0.0f, 0, 2.0f);

    // Render scene content
    renderSceneContent();

    // Render overlays on top
    renderOverlays();

    // Handle drag and drop
    handleDragDrop();

    // Show context menu
    if (ImGui::BeginPopupContextWindow())
    {
        renderMenuItems(getContextMenuItems());
        ImGui::EndPopup();
    }

    // Placeholder text when no scene is loaded
    ImGui::SetCursorPos(ImVec2(m_contentWidth / 2 - 100, m_contentHeight / 2));
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Scene View - No scene loaded");
    ImGui::SetCursorPos(ImVec2(m_contentWidth / 2 - 80, m_contentHeight / 2 + 20));
    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Create or open a scene");
#else
    // Stub rendering
    renderSceneContent();
    renderOverlays();
    handleDragDrop();
#endif
}

void SceneViewPanel::onResize(f32 /*width*/, f32 /*height*/)
{
    // Recalculate view parameters if needed
}

void SceneViewPanel::renderToolbar()
{
    widgets::BeginToolbar("SceneViewToolbar");
    renderToolbarItems(getToolbarItems());

    // Add zoom slider on the right
    // ImGui::SameLine(ImGui::GetContentRegionAvail().x - 150);
    // ImGui::SetNextItemWidth(100);
    // ImGui::SliderFloat("##Zoom", &m_zoom, m_minZoom, m_maxZoom, "%.1fx");
    // ImGui::SameLine();
    // if (ImGui::Button("1:1")) setZoom(1.0f);

    widgets::EndToolbar();
}

void SceneViewPanel::onSelectionChanged(SelectionType type,
                                        const std::vector<SelectionItem>& /*selection*/)
{
    if (type == SelectionType::SceneObject)
    {
        // Highlight selected objects
        // Could auto-frame selection if desired
    }
}

void SceneViewPanel::renderSceneContent()
{
    // Get content region
    // ImVec2 contentMin = ImGui::GetCursorScreenPos();
    // ImVec2 contentMax = ImVec2(contentMin.x + m_contentWidth, contentMin.y + m_contentHeight);
    // ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw scene background
    // drawList->AddRectFilled(contentMin, contentMax, IM_COL32(30, 30, 30, 255));

    // Draw grid if enabled
    if (m_showGrid)
    {
        renderGrid();
    }

    // Draw scene bounds indicator
    // f32 sceneLeft, sceneTop, sceneRight, sceneBottom;
    // std::tie(sceneLeft, sceneTop) = sceneToScreen(0, 0);
    // std::tie(sceneRight, sceneBottom) = sceneToScreen(m_sceneWidth, m_sceneHeight);
    // drawList->AddRect(
    //     ImVec2(sceneLeft, sceneTop),
    //     ImVec2(sceneRight, sceneBottom),
    //     IM_COL32(100, 100, 100, 255), 0.0f, 0, 2.0f);

    // Render scene objects based on render mode
    // This would iterate through scene objects and draw them
    // For now, this is a placeholder for actual scene rendering

    // Render selection highlight
    renderSelectionHighlight();

    // Render transform gizmos for selected objects
    if (getSelection().hasSelectionOfType(SelectionType::SceneObject))
    {
        renderGizmos();
    }
}

void SceneViewPanel::renderGrid()
{
    // Draw grid lines
    // ImDrawList* drawList = ImGui::GetWindowDrawList();
    // ImVec2 contentMin = ImGui::GetCursorScreenPos();

    f32 scaledGridSize = m_gridSize * m_zoom;

    // Only draw if grid isn't too dense or sparse
    if (scaledGridSize < 5.0f || scaledGridSize > 500.0f)
    {
        return;
    }

    // Calculate grid offset based on pan
    // f32 startX = std::fmod(m_panX * m_zoom + m_contentWidth / 2.0f, scaledGridSize);
    // f32 startY = std::fmod(m_panY * m_zoom + m_contentHeight / 2.0f, scaledGridSize);

    // Draw vertical lines
    // for (f32 x = startX; x < m_contentWidth; x += scaledGridSize)
    // {
    //     drawList->AddLine(
    //         ImVec2(contentMin.x + x, contentMin.y),
    //         ImVec2(contentMin.x + x, contentMin.y + m_contentHeight),
    //         IM_COL32(m_gridColor.r, m_gridColor.g, m_gridColor.b, m_gridColor.a));
    // }

    // Draw horizontal lines
    // for (f32 y = startY; y < m_contentHeight; y += scaledGridSize)
    // {
    //     drawList->AddLine(
    //         ImVec2(contentMin.x, contentMin.y + y),
    //         ImVec2(contentMin.x + m_contentWidth, contentMin.y + y),
    //         IM_COL32(m_gridColor.r, m_gridColor.g, m_gridColor.b, m_gridColor.a));
    // }
    (void)scaledGridSize;
}

void SceneViewPanel::renderGizmos()
{
    // Get selected object positions
    auto selectedIds = getSelection().getSelectedObjectIds();
    if (selectedIds.empty())
    {
        return;
    }

    // Calculate gizmo center (average of selected object positions)
    // For now, use placeholder position
    f32 gizmoX = m_contentWidth / 2.0f;
    f32 gizmoY = m_contentHeight / 2.0f;

    // ImDrawList* drawList = ImGui::GetWindowDrawList();
    // ImVec2 contentMin = ImGui::GetCursorScreenPos();
    // ImVec2 gizmoCenter(contentMin.x + gizmoX, contentMin.y + gizmoY);

    f32 gizmoSize = 80.0f;

    switch (m_currentTool)
    {
        case TransformTool::Move:
            // Draw move gizmo (arrows)
            // X axis (red arrow)
            // drawList->AddLine(gizmoCenter,
            //     ImVec2(gizmoCenter.x + gizmoSize, gizmoCenter.y),
            //     IM_COL32(m_gizmoXColor.r, m_gizmoXColor.g, m_gizmoXColor.b, m_gizmoXColor.a), 2.0f);
            // Y axis (green arrow)
            // drawList->AddLine(gizmoCenter,
            //     ImVec2(gizmoCenter.x, gizmoCenter.y - gizmoSize),
            //     IM_COL32(m_gizmoYColor.r, m_gizmoYColor.g, m_gizmoYColor.b, m_gizmoYColor.a), 2.0f);
            // XY plane (yellow square)
            // drawList->AddRectFilled(
            //     ImVec2(gizmoCenter.x + 5, gizmoCenter.y - 25),
            //     ImVec2(gizmoCenter.x + 25, gizmoCenter.y - 5),
            //     IM_COL32(m_gizmoXYColor.r, m_gizmoXYColor.g, m_gizmoXYColor.b, 128));
            break;

        case TransformTool::Rotate:
            // Draw rotate gizmo (circle)
            // drawList->AddCircle(gizmoCenter, gizmoSize,
            //     IM_COL32(m_selectionColor.r, m_selectionColor.g, m_selectionColor.b, m_selectionColor.a), 32, 2.0f);
            break;

        case TransformTool::Scale:
            // Draw scale gizmo (lines with boxes at ends)
            // Similar to move but with squares instead of arrows
            break;

        case TransformTool::Rect:
            // Draw rect transform handles
            // Draw handles at corners and edges
            break;

        default:
            break;
    }

    (void)gizmoX;
    (void)gizmoY;
    (void)gizmoSize;
}

void SceneViewPanel::renderSelectionHighlight()
{
    auto selectedIds = getSelection().getSelectedObjectIds();
    if (selectedIds.empty())
    {
        return;
    }

    // Draw selection rectangles around selected objects
    // This would iterate through selected objects and draw their bounds
    // ImDrawList* drawList = ImGui::GetWindowDrawList();

    // For each selected object:
    // - Get object bounds
    // - Convert to screen coordinates
    // - Draw selection rectangle
}

void SceneViewPanel::renderOverlays()
{
    // Draw zoom indicator
    // char zoomText[32];
    // snprintf(zoomText, sizeof(zoomText), "%.0f%%", m_zoom * 100.0f);
    // ImVec2 textSize = ImGui::CalcTextSize(zoomText);
    // ImGui::SetCursorPos(ImVec2(m_contentWidth - textSize.x - 10, 10));
    // ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", zoomText);

    // Draw coordinate indicator
    // if (m_isHovered)
    // {
    //     auto [sceneX, sceneY] = screenToScene(m_lastMouseX, m_lastMouseY);
    //     ImGui::SetCursorPos(ImVec2(10, m_contentHeight - 30));
    //     ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "X: %.1f Y: %.1f", sceneX, sceneY);
    // }
}

void SceneViewPanel::renderLayerControls()
{
    // Draw layer visibility popup
    // if (ImGui::BeginPopup("LayerControls"))
    // {
    //     for (int i = 0; i < 8; i++)
    //     {
    //         bool visible = isLayerVisible(i);
    //         char label[32];
    //         snprintf(label, sizeof(label), "Layer %d", i);
    //         if (ImGui::Checkbox(label, &visible))
    //         {
    //             setLayerVisible(i, visible);
    //         }
    //     }
    //     ImGui::Separator();
    //     if (ImGui::Button("Show All"))
    //     {
    //         showAllLayers();
    //     }
    //     ImGui::EndPopup();
    // }
}

void SceneViewPanel::handleMouseInput()
{
    if (!m_isHovered)
    {
        return;
    }

    // Get mouse position relative to content area
    // ImVec2 mousePos = ImGui::GetMousePos();
    // ImVec2 contentMin = ImGui::GetCursorScreenPos();
    // f32 localX = mousePos.x - contentMin.x;
    // f32 localY = mousePos.y - contentMin.y;

    // Handle mouse wheel zoom
    // f32 wheel = ImGui::GetIO().MouseWheel;
    // if (wheel != 0.0f)
    // {
    //     f32 zoomFactor = 1.0f + wheel * 0.1f;
    //     setZoom(m_zoom * zoomFactor);
    // }

    // Handle middle mouse panning
    // if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
    // {
    //     if (!m_isPanning)
    //     {
    //         m_isPanning = true;
    //         m_dragStartX = localX;
    //         m_dragStartY = localY;
    //     }
    //     else
    //     {
    //         f32 deltaX = localX - m_lastMouseX;
    //         f32 deltaY = localY - m_lastMouseY;
    //         m_panX += deltaX / m_zoom;
    //         m_panY += deltaY / m_zoom;
    //     }
    // }
    // else
    // {
    //     m_isPanning = false;
    // }

    // Handle left click selection
    // if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    // {
    //     bool addToSelection = ImGui::GetIO().KeyShift;
    //     selectObjectAtPosition(localX, localY, addToSelection);
    // }

    // Handle dragging
    // if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && getSelection().hasSelection())
    // {
    //     f32 deltaX = localX - m_lastMouseX;
    //     f32 deltaY = localY - m_lastMouseY;
    //     moveSelectedObjects(deltaX / m_zoom, deltaY / m_zoom);
    // }

    // m_lastMouseX = localX;
    // m_lastMouseY = localY;
}

void SceneViewPanel::handleKeyboardInput()
{
    if (!m_isFocused)
    {
        return;
    }

    // Tool shortcuts
    // if (ImGui::IsKeyPressed(ImGuiKey_Q)) setCurrentTool(TransformTool::Select);
    // if (ImGui::IsKeyPressed(ImGuiKey_W)) setCurrentTool(TransformTool::Move);
    // if (ImGui::IsKeyPressed(ImGuiKey_E)) setCurrentTool(TransformTool::Rotate);
    // if (ImGui::IsKeyPressed(ImGuiKey_R)) setCurrentTool(TransformTool::Scale);
    // if (ImGui::IsKeyPressed(ImGuiKey_T)) setCurrentTool(TransformTool::Rect);

    // View shortcuts
    // if (ImGui::IsKeyPressed(ImGuiKey_Home)) resetView();
    // if (ImGui::IsKeyPressed(ImGuiKey_F) && ImGui::GetIO().KeyShift) zoomToFit();

    // Delete selected
    // if (ImGui::IsKeyPressed(ImGuiKey_Delete))
    // {
    //     // Delete selected objects
    // }
}

void SceneViewPanel::handleDragDrop()
{
    // Handle drag-drop of assets into scene
    // if (ImGui::BeginDragDropTarget())
    // {
    //     if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
    //     {
    //         const char* assetPath = (const char*)payload->Data;
    //         // Create object from dropped asset
    //     }
    //     ImGui::EndDragDropTarget();
    // }
}

void SceneViewPanel::selectObjectAtPosition(f32 x, f32 y, bool addToSelection)
{
    // Convert screen position to scene coordinates
    auto [sceneX, sceneY] = screenToScene(x, y);

    // Hit test against scene objects
    // This would iterate through objects and check bounds
    // For now, placeholder implementation

    if (!addToSelection)
    {
        getSelection().clearSelection();
    }

    // If object found at position:
    // getSelection().selectObject(objectId);

    (void)sceneX;
    (void)sceneY;
}

void SceneViewPanel::moveSelectedObjects(f32 deltaX, f32 deltaY)
{
    if (m_snapEnabled)
    {
        deltaX = snapValue(deltaX);
        deltaY = snapValue(deltaY);
    }

    auto selectedIds = getSelection().getSelectedObjectIds();
    for ([[maybe_unused]] const auto& id : selectedIds)
    {
        // Move each selected object
        // This would update object transforms through the scene system
    }

    // Publish event
    SceneObjectTransformedEvent event;
    publishEvent(event);

    (void)deltaX;
    (void)deltaY;
}

void SceneViewPanel::rotateSelectedObjects(f32 angle)
{
    if (m_snapEnabled)
    {
        // Snap to 15 degree increments
        angle = std::round(angle / 15.0f) * 15.0f;
    }

    auto selectedIds = getSelection().getSelectedObjectIds();
    for ([[maybe_unused]] const auto& id : selectedIds)
    {
        // Rotate each selected object
    }

    (void)angle;
}

void SceneViewPanel::scaleSelectedObjects(f32 scaleX, f32 scaleY)
{
    auto selectedIds = getSelection().getSelectedObjectIds();
    for ([[maybe_unused]] const auto& id : selectedIds)
    {
        // Scale each selected object
    }

    (void)scaleX;
    (void)scaleY;
}

std::pair<f32, f32> SceneViewPanel::screenToScene(f32 screenX, f32 screenY) const
{
    f32 centerX = m_contentWidth / 2.0f;
    f32 centerY = m_contentHeight / 2.0f;

    f32 sceneX = (screenX - centerX) / m_zoom - m_panX + m_sceneWidth / 2.0f;
    f32 sceneY = (screenY - centerY) / m_zoom - m_panY + m_sceneHeight / 2.0f;

    return {sceneX, sceneY};
}

std::pair<f32, f32> SceneViewPanel::sceneToScreen(f32 sceneX, f32 sceneY) const
{
    f32 centerX = m_contentWidth / 2.0f;
    f32 centerY = m_contentHeight / 2.0f;

    f32 screenX = (sceneX - m_sceneWidth / 2.0f + m_panX) * m_zoom + centerX;
    f32 screenY = (sceneY - m_sceneHeight / 2.0f + m_panY) * m_zoom + centerY;

    return {screenX, screenY};
}

f32 SceneViewPanel::snapValue(f32 value) const
{
    if (!m_snapEnabled || m_snapIncrement <= 0.0f)
    {
        return value;
    }
    return std::round(value / m_snapIncrement) * m_snapIncrement;
}

} // namespace NovelMind::editor
