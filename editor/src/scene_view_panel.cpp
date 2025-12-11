/**
 * @file scene_view_panel.cpp
 * @brief Scene View Panel implementation
 */

#include "NovelMind/editor/scene_view_panel.hpp"
#include "NovelMind/editor/editor_app.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include "NovelMind/scene/scene_graph.hpp"
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

    // Transform tools with enhanced tooltips
    items.push_back({"Q", "Select Tool (Q) - Click to select objects, drag to box select",
                     [this]() { const_cast<SceneViewPanel*>(this)->setCurrentTool(TransformTool::Select); },
                     []() { return true; }, [this]() { return m_currentTool == TransformTool::Select; }});
    items.push_back({"W", "Move Tool (W) - Drag objects to reposition them in the scene",
                     [this]() { const_cast<SceneViewPanel*>(this)->setCurrentTool(TransformTool::Move); },
                     []() { return true; }, [this]() { return m_currentTool == TransformTool::Move; }});
    items.push_back({"E", "Rotate Tool (E) - Drag to rotate selected objects around their center",
                     [this]() { const_cast<SceneViewPanel*>(this)->setCurrentTool(TransformTool::Rotate); },
                     []() { return true; }, [this]() { return m_currentTool == TransformTool::Rotate; }});
    items.push_back({"R", "Scale Tool (R) - Drag handles to resize selected objects",
                     [this]() { const_cast<SceneViewPanel*>(this)->setCurrentTool(TransformTool::Scale); },
                     []() { return true; }, [this]() { return m_currentTool == TransformTool::Scale; }});
    items.push_back({"T", "Rect Tool (T) - Draw or edit rectangular bounds",
                     [this]() { const_cast<SceneViewPanel*>(this)->setCurrentTool(TransformTool::Rect); },
                     []() { return true; }, [this]() { return m_currentTool == TransformTool::Rect; }});

    items.push_back(ToolbarItem::separator());

    // Gizmo space toggle with enhanced tooltip
    items.push_back({"L/G", "Toggle Local/Global Space - Switch between object-relative and world-relative coordinates",
                     [this]() { const_cast<SceneViewPanel*>(this)->toggleGizmoSpace(); },
                     []() { return true; }, [this]() { return m_gizmoSpace == GizmoSpace::Local; }});

    items.push_back(ToolbarItem::separator());

    // View controls with enhanced tooltips
    items.push_back({"#", "Toggle Grid - Show/hide alignment grid for easier positioning",
                     [this]() { const_cast<SceneViewPanel*>(this)->setGridVisible(!m_showGrid); },
                     []() { return true; }, [this]() { return m_showGrid; }});
    items.push_back({"S", "Toggle Snapping - Enable/disable snap-to-grid when moving objects",
                     [this]() { const_cast<SceneViewPanel*>(this)->setSnappingEnabled(!m_snapEnabled); },
                     []() { return true; }, [this]() { return m_snapEnabled; }});

    return items;
}

std::vector<MenuItem> SceneViewPanel::getContextMenuItems() const
{
    std::vector<MenuItem> items;

    bool hasSelection = getSelection().hasSelection();

    items.push_back({"Cut", "Ctrl+X", []() { /* TODO: Cut selected objects to clipboard */ },
                     [hasSelection]() { return hasSelection; }});
    items.push_back({"Copy", "Ctrl+C", []() { /* TODO: Copy selected objects to clipboard */ },
                     [hasSelection]() { return hasSelection; }});
    items.push_back({"Paste", "Ctrl+V", []() { /* TODO: Paste objects from clipboard */ }});
    items.push_back({"Delete", "Delete", []() { /* TODO: Delete selected objects from scene */ },
                     [hasSelection]() { return hasSelection; }});
    items.push_back(MenuItem::separator());
    items.push_back({"Duplicate", "Ctrl+D", []() { /* TODO: Duplicate selected objects */ },
                     [hasSelection]() { return hasSelection; }});
    items.push_back(MenuItem::separator());
    items.push_back({"Create Empty Object", "", []() { /* TODO: Create empty scene object */ }});
    items.push_back({"Create Character Sprite", "", []() { /* TODO: Add new character to scene */ }});
    items.push_back({"Create Background", "", []() { /* TODO: Add new background layer */ }});
    items.push_back({"Create UI Element", "", []() { /* TODO: Add new UI component */ }});

    return items;
}

void SceneViewPanel::onInitialize()
{
    // Get SceneGraph reference from EditorApp
    if (getApp())
    {
        m_sceneGraph = getApp()->getSceneGraph();
    }

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

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Add zoom slider on the right
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 150);
    ImGui::SetNextItemWidth(100);
    ImGui::SliderFloat("##Zoom", &m_zoom, m_minZoom, m_maxZoom, "%.1fx");
    ImGui::SameLine();
    if (ImGui::Button("1:1")) setZoom(1.0f);
#endif

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
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImVec2 contentMin = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw grid if enabled
    if (m_showGrid)
    {
        renderGrid();
    }

    // Draw scene bounds indicator
    f32 sceneLeft, sceneTop, sceneRight, sceneBottom;
    std::tie(sceneLeft, sceneTop) = sceneToScreen(0, 0);
    std::tie(sceneRight, sceneBottom) = sceneToScreen(m_sceneWidth, m_sceneHeight);

    // Offset by content position
    sceneLeft += contentMin.x;
    sceneTop += contentMin.y;
    sceneRight += contentMin.x;
    sceneBottom += contentMin.y;

    // Draw scene bounds border
    drawList->AddRect(
        ImVec2(sceneLeft, sceneTop),
        ImVec2(sceneRight, sceneBottom),
        IM_COL32(100, 100, 100, 255), 0.0f, 0, 2.0f);

    // Render scene objects based on render mode
    // This would iterate through scene objects and draw them
    // For now, draw placeholder objects for demonstration
    renderPlaceholderObjects();

#else
    if (m_showGrid)
    {
        renderGrid();
    }
#endif

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
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentMin = ImGui::GetCursorScreenPos();

    f32 scaledGridSize = m_gridSize * m_zoom;

    // Only draw if grid isn't too dense or sparse
    if (scaledGridSize < 5.0f || scaledGridSize > 500.0f)
    {
        return;
    }

    // Calculate grid offset based on pan
    f32 startX = std::fmod(m_panX * m_zoom + m_contentWidth / 2.0f, scaledGridSize);
    f32 startY = std::fmod(m_panY * m_zoom + m_contentHeight / 2.0f, scaledGridSize);
    if (startX < 0) startX += scaledGridSize;
    if (startY < 0) startY += scaledGridSize;

    ImU32 gridColor = IM_COL32(m_gridColor.r, m_gridColor.g, m_gridColor.b, m_gridColor.a);
    ImU32 majorGridColor = IM_COL32(80, 80, 80, 150);

    // Draw vertical lines
    int lineIdx = 0;
    for (f32 x = startX; x < m_contentWidth; x += scaledGridSize)
    {
        bool isMajor = (lineIdx % 5 == 0);
        drawList->AddLine(
            ImVec2(contentMin.x + x, contentMin.y),
            ImVec2(contentMin.x + x, contentMin.y + m_contentHeight),
            isMajor ? majorGridColor : gridColor, isMajor ? 1.5f : 1.0f);
        lineIdx++;
    }

    // Draw horizontal lines
    lineIdx = 0;
    for (f32 y = startY; y < m_contentHeight; y += scaledGridSize)
    {
        bool isMajor = (lineIdx % 5 == 0);
        drawList->AddLine(
            ImVec2(contentMin.x, contentMin.y + y),
            ImVec2(contentMin.x + m_contentWidth, contentMin.y + y),
            isMajor ? majorGridColor : gridColor, isMajor ? 1.5f : 1.0f);
        lineIdx++;
    }
#else
    f32 scaledGridSize = m_gridSize * m_zoom;
    (void)scaledGridSize;
#endif
}

void SceneViewPanel::renderGizmos()
{
    auto selectedIds = getSelection().getSelectedObjectIds();
    if (selectedIds.empty())
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentMin = ImGui::GetCursorScreenPos();

    // Calculate gizmo center (would use actual object position in real implementation)
    f32 gizmoX = m_contentWidth / 2.0f;
    f32 gizmoY = m_contentHeight / 2.0f;
    ImVec2 gizmoCenter(contentMin.x + gizmoX, contentMin.y + gizmoY);

    f32 gizmoSize = 80.0f;
    f32 arrowHeadSize = 10.0f;

    ImU32 xColor = IM_COL32(m_gizmoXColor.r, m_gizmoXColor.g, m_gizmoXColor.b, m_gizmoXColor.a);
    ImU32 yColor = IM_COL32(m_gizmoYColor.r, m_gizmoYColor.g, m_gizmoYColor.b, m_gizmoYColor.a);
    ImU32 xyColor = IM_COL32(m_gizmoXYColor.r, m_gizmoXYColor.g, m_gizmoXYColor.b, 128);

    switch (m_currentTool)
    {
        case TransformTool::Move:
        {
            // X axis (red arrow)
            drawList->AddLine(gizmoCenter,
                ImVec2(gizmoCenter.x + gizmoSize, gizmoCenter.y),
                xColor, 3.0f);
            // X arrow head
            drawList->AddTriangleFilled(
                ImVec2(gizmoCenter.x + gizmoSize + arrowHeadSize, gizmoCenter.y),
                ImVec2(gizmoCenter.x + gizmoSize - 5, gizmoCenter.y - arrowHeadSize/2),
                ImVec2(gizmoCenter.x + gizmoSize - 5, gizmoCenter.y + arrowHeadSize/2),
                xColor);

            // Y axis (green arrow pointing up)
            drawList->AddLine(gizmoCenter,
                ImVec2(gizmoCenter.x, gizmoCenter.y - gizmoSize),
                yColor, 3.0f);
            // Y arrow head
            drawList->AddTriangleFilled(
                ImVec2(gizmoCenter.x, gizmoCenter.y - gizmoSize - arrowHeadSize),
                ImVec2(gizmoCenter.x - arrowHeadSize/2, gizmoCenter.y - gizmoSize + 5),
                ImVec2(gizmoCenter.x + arrowHeadSize/2, gizmoCenter.y - gizmoSize + 5),
                yColor);

            // XY plane handle (yellow square)
            drawList->AddRectFilled(
                ImVec2(gizmoCenter.x + 10, gizmoCenter.y - 30),
                ImVec2(gizmoCenter.x + 30, gizmoCenter.y - 10),
                xyColor);
            break;
        }

        case TransformTool::Rotate:
        {
            // Draw rotation circle
            drawList->AddCircle(gizmoCenter, gizmoSize,
                IM_COL32(m_selectionColor.r, m_selectionColor.g, m_selectionColor.b, 200), 48, 2.5f);

            // Draw rotation angle indicator
            f32 angle = 0.0f; // Would be actual rotation
            f32 indicatorX = gizmoCenter.x + std::cos(angle) * gizmoSize;
            f32 indicatorY = gizmoCenter.y - std::sin(angle) * gizmoSize;
            drawList->AddLine(gizmoCenter, ImVec2(indicatorX, indicatorY),
                IM_COL32(255, 255, 255, 255), 2.0f);
            drawList->AddCircleFilled(ImVec2(indicatorX, indicatorY), 5.0f,
                IM_COL32(255, 255, 255, 255));
            break;
        }

        case TransformTool::Scale:
        {
            // X axis scale (red with box)
            drawList->AddLine(gizmoCenter,
                ImVec2(gizmoCenter.x + gizmoSize, gizmoCenter.y),
                xColor, 3.0f);
            drawList->AddRectFilled(
                ImVec2(gizmoCenter.x + gizmoSize - 5, gizmoCenter.y - 5),
                ImVec2(gizmoCenter.x + gizmoSize + 5, gizmoCenter.y + 5),
                xColor);

            // Y axis scale (green with box)
            drawList->AddLine(gizmoCenter,
                ImVec2(gizmoCenter.x, gizmoCenter.y - gizmoSize),
                yColor, 3.0f);
            drawList->AddRectFilled(
                ImVec2(gizmoCenter.x - 5, gizmoCenter.y - gizmoSize - 5),
                ImVec2(gizmoCenter.x + 5, gizmoCenter.y - gizmoSize + 5),
                yColor);

            // Uniform scale (center box)
            drawList->AddRectFilled(
                ImVec2(gizmoCenter.x - 6, gizmoCenter.y - 6),
                ImVec2(gizmoCenter.x + 6, gizmoCenter.y + 6),
                IM_COL32(255, 255, 255, 200));
            break;
        }

        case TransformTool::Rect:
        {
            // Draw rect transform with 8 handles (corners + edges)
            f32 rectSize = 100.0f;
            ImVec2 topLeft(gizmoCenter.x - rectSize/2, gizmoCenter.y - rectSize/2);
            ImVec2 bottomRight(gizmoCenter.x + rectSize/2, gizmoCenter.y + rectSize/2);

            // Rect outline
            drawList->AddRect(topLeft, bottomRight,
                IM_COL32(m_selectionColor.r, m_selectionColor.g, m_selectionColor.b, 255), 0.0f, 0, 2.0f);

            // Corner handles
            f32 handleSize = 6.0f;
            ImU32 handleColor = IM_COL32(255, 255, 255, 255);
            // Top-left
            drawList->AddRectFilled(
                ImVec2(topLeft.x - handleSize, topLeft.y - handleSize),
                ImVec2(topLeft.x + handleSize, topLeft.y + handleSize), handleColor);
            // Top-right
            drawList->AddRectFilled(
                ImVec2(bottomRight.x - handleSize, topLeft.y - handleSize),
                ImVec2(bottomRight.x + handleSize, topLeft.y + handleSize), handleColor);
            // Bottom-left
            drawList->AddRectFilled(
                ImVec2(topLeft.x - handleSize, bottomRight.y - handleSize),
                ImVec2(topLeft.x + handleSize, bottomRight.y + handleSize), handleColor);
            // Bottom-right
            drawList->AddRectFilled(
                ImVec2(bottomRight.x - handleSize, bottomRight.y - handleSize),
                ImVec2(bottomRight.x + handleSize, bottomRight.y + handleSize), handleColor);

            // Edge handles
            // Top
            drawList->AddRectFilled(
                ImVec2(gizmoCenter.x - handleSize, topLeft.y - handleSize),
                ImVec2(gizmoCenter.x + handleSize, topLeft.y + handleSize), handleColor);
            // Bottom
            drawList->AddRectFilled(
                ImVec2(gizmoCenter.x - handleSize, bottomRight.y - handleSize),
                ImVec2(gizmoCenter.x + handleSize, bottomRight.y + handleSize), handleColor);
            // Left
            drawList->AddRectFilled(
                ImVec2(topLeft.x - handleSize, gizmoCenter.y - handleSize),
                ImVec2(topLeft.x + handleSize, gizmoCenter.y + handleSize), handleColor);
            // Right
            drawList->AddRectFilled(
                ImVec2(bottomRight.x - handleSize, gizmoCenter.y - handleSize),
                ImVec2(bottomRight.x + handleSize, gizmoCenter.y + handleSize), handleColor);
            break;
        }

        default:
            break;
    }
#endif
}

void SceneViewPanel::renderPlaceholderObjects()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // If we have a SceneGraph, render real objects from it
    if (m_sceneGraph)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 contentMin = ImGui::GetCursorScreenPos();

        // Helper lambda to render a scene object
        auto renderSceneObject = [&](const scene::SceneObjectBase* obj) {
            if (!obj || !obj->isVisible()) return;

            // Get object bounds
            f32 x = obj->getX();
            f32 y = obj->getY();
            f32 scaleX = obj->getScaleX();
            f32 scaleY = obj->getScaleY();
            f32 alpha = obj->getAlpha();

            // Assume default object size (can be overridden per object type)
            f32 width = 200.0f * scaleX;
            f32 height = 300.0f * scaleY;

            // Convert to screen coordinates
            auto [screenX, screenY] = sceneToScreen(x - width/2, y - height/2);
            auto [screenX2, screenY2] = sceneToScreen(x + width/2, y + height/2);

            // Choose color based on object type
            ImU32 fillColor;
            const char* label = obj->getTypeName();

            switch (obj->getType())
            {
                case scene::SceneObjectType::Background:
                    fillColor = IM_COL32(60, 80, 100, static_cast<u8>(100 * alpha));
                    break;
                case scene::SceneObjectType::Character:
                    fillColor = IM_COL32(180, 140, 100, static_cast<u8>(200 * alpha));
                    break;
                case scene::SceneObjectType::DialogueUI:
                    fillColor = IM_COL32(40, 40, 40, static_cast<u8>(220 * alpha));
                    break;
                case scene::SceneObjectType::ChoiceUI:
                    fillColor = IM_COL32(60, 60, 80, static_cast<u8>(220 * alpha));
                    break;
                default:
                    fillColor = IM_COL32(100, 100, 100, static_cast<u8>(150 * alpha));
                    break;
            }

            // Draw object bounds
            drawList->AddRectFilled(
                ImVec2(contentMin.x + screenX, contentMin.y + screenY),
                ImVec2(contentMin.x + screenX2, contentMin.y + screenY2),
                fillColor);

            drawList->AddRect(
                ImVec2(contentMin.x + screenX, contentMin.y + screenY),
                ImVec2(contentMin.x + screenX2, contentMin.y + screenY2),
                IM_COL32(120, 120, 120, 255), 0.0f, 0, 1.0f);

            // Draw object label
            std::string displayText = std::string(label) + ": " + obj->getId();
            drawList->AddText(
                ImVec2(contentMin.x + screenX + 5, contentMin.y + screenY + 5),
                IM_COL32(255, 255, 255, 255),
                displayText.c_str());
        };

        // Render objects from all layers
        // Background layer
        for (const auto& obj : m_sceneGraph->getBackgroundLayer().getObjects())
        {
            renderSceneObject(obj.get());
        }

        // Character layer
        for (const auto& obj : m_sceneGraph->getCharacterLayer().getObjects())
        {
            renderSceneObject(obj.get());
        }

        // UI layer
        for (const auto& obj : m_sceneGraph->getUILayer().getObjects())
        {
            renderSceneObject(obj.get());
        }

        // Effect layer
        for (const auto& obj : m_sceneGraph->getEffectLayer().getObjects())
        {
            renderSceneObject(obj.get());
        }
    }
    else
    {
        // Fallback: render placeholder if no SceneGraph available
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 contentMin = ImGui::GetCursorScreenPos();

        // Draw empty scene message
        auto [msgX, msgY] = sceneToScreen(m_sceneWidth/2 - 150, m_sceneHeight/2);
        drawList->AddText(
            ImVec2(contentMin.x + msgX, contentMin.y + msgY),
            IM_COL32(150, 150, 150, 255),
            "No scene loaded. Create or open a project.");
    }
#endif
}

void SceneViewPanel::renderSelectionHighlight()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    auto selectedIds = getSelection().getSelectedObjectIds();
    if (selectedIds.empty())
    {
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentMin = ImGui::GetCursorScreenPos();

    ImU32 selectionColor = IM_COL32(m_selectionColor.r, m_selectionColor.g, m_selectionColor.b, 255);
    ImU32 selectionFillColor = IM_COL32(m_selectionColor.r, m_selectionColor.g, m_selectionColor.b, 30);

    // For each selected object, draw selection rectangle
    // TODO: This needs to be connected to actual scene graph
    // For now, draw a placeholder selection box at the center
    for (size_t i = 0; i < selectedIds.size(); ++i)
    {
        // Placeholder: draw selection at center with offset per object
        f32 objX = m_sceneWidth / 2.0f + (static_cast<f32>(i) * 50.0f);
        f32 objY = m_sceneHeight / 2.0f;
        f32 objWidth = 200.0f;
        f32 objHeight = 300.0f;

        // Convert object bounds to screen coordinates
        auto [screenX1, screenY1] = sceneToScreen(objX - objWidth/2, objY - objHeight/2);
        auto [screenX2, screenY2] = sceneToScreen(objX + objWidth/2, objY + objHeight/2);

        screenX1 += contentMin.x;
        screenY1 += contentMin.y;
        screenX2 += contentMin.x;
        screenY2 += contentMin.y;

        // Draw selection fill
        drawList->AddRectFilled(
            ImVec2(screenX1, screenY1),
            ImVec2(screenX2, screenY2),
            selectionFillColor);

        // Draw selection border
        drawList->AddRect(
            ImVec2(screenX1, screenY1),
            ImVec2(screenX2, screenY2),
            selectionColor, 0.0f, 0, 2.5f);

        // Draw corner handles for selected objects
        f32 handleSize = 8.0f;
        ImU32 handleColor = IM_COL32(255, 255, 255, 255);

        // Top-left
        drawList->AddRectFilled(
            ImVec2(screenX1 - handleSize/2, screenY1 - handleSize/2),
            ImVec2(screenX1 + handleSize/2, screenY1 + handleSize/2),
            handleColor);
        // Top-right
        drawList->AddRectFilled(
            ImVec2(screenX2 - handleSize/2, screenY1 - handleSize/2),
            ImVec2(screenX2 + handleSize/2, screenY1 + handleSize/2),
            handleColor);
        // Bottom-left
        drawList->AddRectFilled(
            ImVec2(screenX1 - handleSize/2, screenY2 - handleSize/2),
            ImVec2(screenX1 + handleSize/2, screenY2 + handleSize/2),
            handleColor);
        // Bottom-right
        drawList->AddRectFilled(
            ImVec2(screenX2 - handleSize/2, screenY2 - handleSize/2),
            ImVec2(screenX2 + handleSize/2, screenY2 + handleSize/2),
            handleColor);
    }
#else
    auto selectedIds = getSelection().getSelectedObjectIds();
    (void)selectedIds;
#endif
}

void SceneViewPanel::renderOverlays()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    // Draw zoom indicator (top-right corner)
    char zoomText[32];
    snprintf(zoomText, sizeof(zoomText), "%.0f%%", m_zoom * 100.0f);
    ImVec2 textSize = ImGui::CalcTextSize(zoomText);
    ImVec2 zoomPos(windowPos.x + windowSize.x - textSize.x - 15, windowPos.y + ImGui::GetFrameHeight() + 10);

    // Draw background for zoom text
    drawList->AddRectFilled(
        ImVec2(zoomPos.x - 5, zoomPos.y - 2),
        ImVec2(zoomPos.x + textSize.x + 5, zoomPos.y + textSize.y + 2),
        IM_COL32(0, 0, 0, 180));
    drawList->AddText(zoomPos, IM_COL32(200, 200, 200, 255), zoomText);

    // Draw coordinate indicator (bottom-left corner)
    if (m_isHovered && m_lastMouseX >= 0 && m_lastMouseY >= 0)
    {
        auto [sceneX, sceneY] = screenToScene(m_lastMouseX, m_lastMouseY);
        char coordText[64];
        snprintf(coordText, sizeof(coordText), "X: %.1f Y: %.1f", sceneX, sceneY);

        ImVec2 coordTextSize = ImGui::CalcTextSize(coordText);
        ImVec2 coordPos(windowPos.x + 10, windowPos.y + windowSize.y - coordTextSize.y - 10);

        // Draw background for coordinate text
        drawList->AddRectFilled(
            ImVec2(coordPos.x - 5, coordPos.y - 2),
            ImVec2(coordPos.x + coordTextSize.x + 5, coordPos.y + coordTextSize.y + 2),
            IM_COL32(0, 0, 0, 180));
        drawList->AddText(coordPos, IM_COL32(200, 200, 200, 255), coordText);
    }

    // Draw current tool indicator (bottom-right corner)
    const char* toolName = "";
    switch (m_currentTool)
    {
        case TransformTool::Select: toolName = "Select (Q)"; break;
        case TransformTool::Move: toolName = "Move (W)"; break;
        case TransformTool::Rotate: toolName = "Rotate (E)"; break;
        case TransformTool::Scale: toolName = "Scale (R)"; break;
        case TransformTool::Rect: toolName = "Rect (T)"; break;
    }

    if (toolName[0] != '\0')
    {
        ImVec2 toolTextSize = ImGui::CalcTextSize(toolName);
        ImVec2 toolPos(windowPos.x + windowSize.x - toolTextSize.x - 15,
                       windowPos.y + windowSize.y - toolTextSize.y - 10);

        // Draw background
        drawList->AddRectFilled(
            ImVec2(toolPos.x - 5, toolPos.y - 2),
            ImVec2(toolPos.x + toolTextSize.x + 5, toolPos.y + toolTextSize.y + 2),
            IM_COL32(0, 0, 0, 180));
        drawList->AddText(toolPos, IM_COL32(200, 200, 200, 255), toolName);
    }

    // Draw grid/snap indicators if enabled
    if (m_showGrid || m_snapEnabled)
    {
        char statusText[64] = "";
        if (m_showGrid && m_snapEnabled)
        {
            snprintf(statusText, sizeof(statusText), "Grid: ON | Snap: ON (%.0f)", m_snapIncrement);
        }
        else if (m_showGrid)
        {
            snprintf(statusText, sizeof(statusText), "Grid: ON");
        }
        else if (m_snapEnabled)
        {
            snprintf(statusText, sizeof(statusText), "Snap: ON (%.0f)", m_snapIncrement);
        }

        if (statusText[0] != '\0')
        {
            ImVec2 statusSize = ImGui::CalcTextSize(statusText);
            ImVec2 statusPos(windowPos.x + 10, windowPos.y + ImGui::GetFrameHeight() + 10);

            drawList->AddRectFilled(
                ImVec2(statusPos.x - 5, statusPos.y - 2),
                ImVec2(statusPos.x + statusSize.x + 5, statusPos.y + statusSize.y + 2),
                IM_COL32(0, 0, 0, 180));
            drawList->AddText(statusPos, IM_COL32(180, 220, 180, 255), statusText);
        }
    }
#endif
}

void SceneViewPanel::renderLayerControls()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Draw layer visibility popup
    if (ImGui::BeginPopup("LayerControls"))
    {
        for (int i = 0; i < 8; i++)
        {
            bool visible = isLayerVisible(static_cast<u32>(i));
            char label[32];
            snprintf(label, sizeof(label), "Layer %d", i);
            if (ImGui::Checkbox(label, &visible))
            {
                setLayerVisible(static_cast<u32>(i), visible);
            }
        }
        ImGui::Separator();
        if (ImGui::Button("Show All"))
        {
            showAllLayers();
        }
        ImGui::EndPopup();
    }
#endif
}

void SceneViewPanel::handleMouseInput()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_isHovered)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 contentMin = ImGui::GetWindowPos();
    contentMin.y += ImGui::GetFrameHeight(); // Account for title bar
    f32 localX = mousePos.x - contentMin.x;
    f32 localY = mousePos.y - contentMin.y;

    // Handle mouse wheel zoom
    f32 wheel = io.MouseWheel;
    if (wheel != 0.0f && ImGui::IsWindowHovered())
    {
        f32 zoomFactor = 1.0f + wheel * 0.1f;
        f32 newZoom = m_zoom * zoomFactor;

        // Zoom towards mouse position
        f32 mouseSceneX, mouseSceneY;
        std::tie(mouseSceneX, mouseSceneY) = screenToScene(localX, localY);

        setZoom(newZoom);

        // Adjust pan to keep mouse position steady
        f32 newMouseX, newMouseY;
        std::tie(newMouseX, newMouseY) = sceneToScreen(mouseSceneX, mouseSceneY);
        m_panX += (localX - newMouseX) / m_zoom;
        m_panY += (localY - newMouseY) / m_zoom;
    }

    // Handle middle mouse panning
    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle) && ImGui::IsWindowHovered())
    {
        if (!m_isPanning)
        {
            m_isPanning = true;
            m_dragStartX = localX;
            m_dragStartY = localY;
        }
        else
        {
            f32 deltaX = localX - m_lastMouseX;
            f32 deltaY = localY - m_lastMouseY;
            m_panX += deltaX / m_zoom;
            m_panY += deltaY / m_zoom;
        }
    }
    else
    {
        m_isPanning = false;
    }

    // Handle left click selection
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
    {
        bool addToSelection = io.KeyShift;
        selectObjectAtPosition(localX, localY, addToSelection);
    }

    // Handle dragging selected objects
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && getSelection().hasSelection() &&
        !m_isPanning && ImGui::IsWindowHovered())
    {
        if (m_lastMouseX >= 0 && m_lastMouseY >= 0)
        {
            f32 deltaX = localX - m_lastMouseX;
            f32 deltaY = localY - m_lastMouseY;
            moveSelectedObjects(deltaX / m_zoom, deltaY / m_zoom);
        }
    }

    m_lastMouseX = localX;
    m_lastMouseY = localY;
#else
    (void)m_isHovered;
#endif
}

void SceneViewPanel::handleKeyboardInput()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_isFocused)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    // Tool shortcuts
    if (ImGui::IsKeyPressed(ImGuiKey_Q)) setCurrentTool(TransformTool::Select);
    if (ImGui::IsKeyPressed(ImGuiKey_W)) setCurrentTool(TransformTool::Move);
    if (ImGui::IsKeyPressed(ImGuiKey_E)) setCurrentTool(TransformTool::Rotate);
    if (ImGui::IsKeyPressed(ImGuiKey_R)) setCurrentTool(TransformTool::Scale);
    if (ImGui::IsKeyPressed(ImGuiKey_T)) setCurrentTool(TransformTool::Rect);

    // View shortcuts
    if (ImGui::IsKeyPressed(ImGuiKey_Home)) resetView();
    if (ImGui::IsKeyPressed(ImGuiKey_F) && io.KeyShift) zoomToFit();

    // Toggle grid and snapping
    if (ImGui::IsKeyPressed(ImGuiKey_G)) setGridVisible(!m_showGrid);
    if (ImGui::IsKeyPressed(ImGuiKey_S) && io.KeyCtrl) setSnappingEnabled(!m_snapEnabled);

    // Delete selected objects
    if (ImGui::IsKeyPressed(ImGuiKey_Delete) || (ImGui::IsKeyPressed(ImGuiKey_Backspace) && io.KeyCtrl))
    {
        auto selectedIds = getSelection().getSelectedObjectIds();
        for (const auto& id : selectedIds)
        {
            // TODO: Delete object through scene system
            // This should trigger Undo/Redo and event publication
            (void)id;
        }
        if (!selectedIds.empty())
        {
            getSelection().clearSelection();
            SceneModifiedEvent event;
            publishEvent(event);
        }
    }

    // Duplicate selected (Ctrl+D)
    if (ImGui::IsKeyPressed(ImGuiKey_D) && io.KeyCtrl && !io.KeyShift)
    {
        auto selectedIds = getSelection().getSelectedObjectIds();
        if (!selectedIds.empty())
        {
            // TODO: Duplicate objects through scene system
            // This should trigger Undo/Redo and event publication
        }
    }

    // Select all (Ctrl+A)
    if (ImGui::IsKeyPressed(ImGuiKey_A) && io.KeyCtrl)
    {
        // TODO: Select all scene objects
    }
#else
    (void)m_isFocused;
#endif
}

void SceneViewPanel::handleDragDrop()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Handle drag-drop of assets into scene
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
        {
            const char* assetPath = static_cast<const char*>(payload->Data);

            // Get drop position in scene coordinates
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 contentMin = ImGui::GetWindowPos();
            contentMin.y += ImGui::GetFrameHeight();
            f32 localX = mousePos.x - contentMin.x;
            f32 localY = mousePos.y - contentMin.y;

            auto [sceneX, sceneY] = screenToScene(localX, localY);

            // TODO: Create scene object at position from asset
            // Determine asset type and create appropriate object
            // This should trigger Undo/Redo and event publication
            (void)assetPath;
            (void)sceneX;
            (void)sceneY;

            SceneModifiedEvent event;
            publishEvent(event);
        }

        // Support other drag-drop types
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_OBJECT"))
        {
            // Dragging object from hierarchy - potentially reparent or move
            const char* objectId = static_cast<const char*>(payload->Data);
            (void)objectId;
        }

        ImGui::EndDragDropTarget();
    }
#endif
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
