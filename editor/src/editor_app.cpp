/**
 * @file editor_app.cpp
 * @brief NovelMind Editor Application implementation
 */

#include "NovelMind/editor/editor_app.hpp"
#include "NovelMind/core/logger.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>

// Include ImGui for menu rendering
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
#include <imgui.h>
#endif

namespace NovelMind::editor
{

namespace fs = std::filesystem;

// ============================================================================
// EditorPanel
// ============================================================================

EditorPanel::EditorPanel(const std::string& title)
    : m_title(title)
{
}

// ============================================================================
// ProjectBrowser
// ============================================================================

ProjectBrowser::ProjectBrowser()
    : EditorPanel("Project")
{
}

void ProjectBrowser::update(f64 /*deltaTime*/)
{
    // Update file watcher, check for changes
}

void ProjectBrowser::render()
{
    if (!m_visible) return;

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    bool open = m_visible;
    if (ImGui::Begin(m_title.c_str(), &open))
    {
        if (m_rootPath.empty())
        {
            ImGui::TextDisabled("Проект не открыт. Файл -> New/Open.");
        }
        else
        {
            ImGui::Text("Root: %s", m_rootPath.c_str());
            ImGui::Separator();

            for (const auto& entry : m_entries)
            {
                bool isDir = entry.type == AssetEntry::Type::Folder;
                std::string label = std::string(isDir ? "[dir] " : "[file] ") + entry.name;

                if (ImGui::Selectable(label.c_str(), m_selectedPath == entry.path, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    m_selectedPath = entry.path;
                    if (m_onFileSelected) m_onFileSelected(entry.path);

                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        if (isDir)
                        {
                            setRootPath(entry.path);
                        }
                        else if (m_onFileDoubleClicked)
                        {
                            m_onFileDoubleClicked(entry.path);
                        }
                    }
                }
            }
        }
    }
    m_visible = open;
    ImGui::End();
#endif
}

void ProjectBrowser::onResize(i32 width, i32 height)
{
    m_width = width;
    m_height = height;
}

void ProjectBrowser::setRootPath(const std::string& path)
{
    m_rootPath = path;
    refresh();
}

void ProjectBrowser::refresh()
{
    m_entries.clear();
    if (!m_rootPath.empty())
    {
        scanDirectory(m_rootPath);
    }
}

void ProjectBrowser::setOnFileSelected(std::function<void(const std::string&)> callback)
{
    m_onFileSelected = std::move(callback);
}

void ProjectBrowser::setOnFileDoubleClicked(std::function<void(const std::string&)> callback)
{
    m_onFileDoubleClicked = std::move(callback);
}

void ProjectBrowser::scanDirectory(const std::string& path)
{
    try
    {
        for (const auto& entry : fs::directory_iterator(path))
        {
            AssetEntry asset;
            asset.name = entry.path().filename().string();
            asset.path = entry.path().string();
            asset.relativePath = fs::relative(entry.path(), m_rootPath).string();

            if (entry.is_directory())
            {
                asset.type = AssetEntry::Type::Folder;
            }
            else
            {
                asset.type = determineAssetType(entry.path().extension().string());
                asset.size = entry.file_size();
            }

            auto modTime = entry.last_write_time();
            asset.modifiedTimestamp = static_cast<u64>(std::chrono::duration_cast<std::chrono::seconds>(
                modTime.time_since_epoch()).count());

            m_entries.push_back(std::move(asset));
        }

        // Sort: folders first, then alphabetically
        std::sort(m_entries.begin(), m_entries.end(),
            [](const AssetEntry& a, const AssetEntry& b)
            {
                if (a.type == AssetEntry::Type::Folder && b.type != AssetEntry::Type::Folder)
                    return true;
                if (a.type != AssetEntry::Type::Folder && b.type == AssetEntry::Type::Folder)
                    return false;
                return a.name < b.name;
            });
    }
    catch (const fs::filesystem_error&)
    {
        // Handle filesystem errors gracefully
    }
}

AssetEntry::Type ProjectBrowser::determineAssetType(const std::string& extension)
{
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".gif")
        return AssetEntry::Type::Image;
    if (ext == ".mp3" || ext == ".wav" || ext == ".ogg" || ext == ".flac")
        return AssetEntry::Type::Audio;
    if (ext == ".nms" || ext == ".nm")
        return AssetEntry::Type::Script;
    if (ext == ".ttf" || ext == ".otf")
        return AssetEntry::Type::Font;
    if (ext == ".mp4" || ext == ".webm" || ext == ".avi")
        return AssetEntry::Type::Video;
    if (ext == ".json" || ext == ".xml" || ext == ".yaml")
        return AssetEntry::Type::Data;

    return AssetEntry::Type::Unknown;
}

// ============================================================================
// SceneView
// ============================================================================

SceneView::SceneView()
    : EditorPanel("Scene")
{
}

SceneView::~SceneView()
{
    if (m_inspector)
    {
        m_inspector->removeListener(this);
    }
}

void SceneView::update(f64 /*deltaTime*/)
{
    // Update scene view state
}

void SceneView::render()
{
    if (!m_visible) return;

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    bool open = m_visible;
    if (ImGui::Begin(m_title.c_str(), &open))
    {
        const auto objectCount = m_sceneGraph ? m_sceneGraph->saveState().objects.size() : 0;
        ImGui::Text("Scene view placeholder");
        ImGui::TextDisabled("Objects in scene: %zu", objectCount);
        ImGui::Separator();
        ImGui::TextDisabled("Use File -> New Scene to start editing.");
    }
    m_visible = open;
    ImGui::End();
#else
    renderGrid();
    renderObjects();
    renderSelection();
    renderGizmos();
#endif
}

void SceneView::onResize(i32 width, i32 height)
{
    m_width = width;
    m_height = height;
}

void SceneView::setSceneGraph(scene::SceneGraph* sceneGraph)
{
    m_sceneGraph = sceneGraph;
}

void SceneView::setInspectorAPI(scene::SceneInspectorAPI* inspector)
{
    if (m_inspector)
    {
        m_inspector->removeListener(this);
    }
    m_inspector = inspector;
    if (m_inspector)
    {
        m_inspector->addListener(this);
    }
}

void SceneView::setViewportOffset(f32 x, f32 y)
{
    m_viewportX = x;
    m_viewportY = y;
}

void SceneView::setViewportZoom(f32 zoom)
{
    m_viewportZoom = std::clamp(zoom, 0.1f, 10.0f);
}

void SceneView::centerViewport()
{
    m_viewportX = 0.0f;
    m_viewportY = 0.0f;
}

void SceneView::setShowGrid(bool show)
{
    m_showGrid = show;
}

void SceneView::setGridSize(f32 size)
{
    m_gridSize = size;
}

void SceneView::setSnapToGrid(bool snap)
{
    m_snapToGrid = snap;
}

void SceneView::selectObjectAt(f32 x, f32 y)
{
    if (!m_sceneGraph || !m_inspector) return;

    // Convert screen to world coordinates
    f32 worldX = (x - m_viewportX) / m_viewportZoom;
    f32 worldY = (y - m_viewportY) / m_viewportZoom;

    // Find object at position (reverse order for top-most first)
    auto state = m_sceneGraph->saveState();
    for (auto it = state.objects.rbegin(); it != state.objects.rend(); ++it)
    {
        const auto& obj = *it;
        if (obj.visible)
        {
            // Simple bounding box check
            f32 halfW = obj.width * obj.scaleX * 0.5f;
            f32 halfH = obj.height * obj.scaleY * 0.5f;

            if (worldX >= obj.x - halfW && worldX <= obj.x + halfW &&
                worldY >= obj.y - halfH && worldY <= obj.y + halfH)
            {
                m_inspector->selectObject(obj.id);
                return;
            }
        }
    }

    // Nothing hit, clear selection
    m_inspector->clearSelection();
}

void SceneView::boxSelect(f32 x1, f32 y1, f32 x2, f32 y2)
{
    if (!m_sceneGraph || !m_inspector) return;

    // Normalize box
    f32 minX = std::min(x1, x2);
    f32 maxX = std::max(x1, x2);
    f32 minY = std::min(y1, y2);
    f32 maxY = std::max(y1, y2);

    // Convert to world coordinates
    minX = (minX - m_viewportX) / m_viewportZoom;
    maxX = (maxX - m_viewportX) / m_viewportZoom;
    minY = (minY - m_viewportY) / m_viewportZoom;
    maxY = (maxY - m_viewportY) / m_viewportZoom;

    m_inspector->clearSelection();

    auto state = m_sceneGraph->saveState();
    for (const auto& obj : state.objects)
    {
        if (obj.visible && obj.x >= minX && obj.x <= maxX &&
            obj.y >= minY && obj.y <= maxY)
        {
            m_inspector->selectObject(obj.id, true);
        }
    }
}

void SceneView::onSelectionChanged(const std::vector<std::string>& /*selectedIds*/)
{
    // Refresh view to show selection
}

void SceneView::onSceneModified()
{
    // Refresh view to show changes
}

void SceneView::onUndoStackChanged(bool /*canUndo*/, bool /*canRedo*/)
{
    // Update toolbar buttons
}

void SceneView::renderGrid()
{
    if (!m_showGrid) return;

    // Render grid lines based on viewport and zoom
    // This would use the renderer to draw grid lines
}

void SceneView::renderObjects()
{
    if (!m_sceneGraph) return;

    // Render all visible objects
    auto state = m_sceneGraph->saveState();
    for (const auto& obj : state.objects)
    {
        if (obj.visible)
        {
            // Transform and render object
            // This would use the renderer to draw objects
        }
    }
}

void SceneView::renderSelection()
{
    if (!m_inspector) return;

    // Render selection handles around selected objects
    const auto& selection = m_inspector->getSelection();
    for (const auto& id : selection)
    {
        auto desc = m_inspector->getObject(id);
        if (desc)
        {
            // Draw selection rectangle and handles
        }
    }
}

void SceneView::renderGizmos()
{
    // Render transform gizmos for selected objects
}

// ============================================================================
// StoryGraphView
// ============================================================================

StoryGraphView::StoryGraphView()
    : EditorPanel("Story")
{
}

void StoryGraphView::update(f64 /*deltaTime*/)
{
    // Update story graph view state
}

void StoryGraphView::render()
{
    if (!m_visible) return;

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    bool open = m_visible;
    if (ImGui::Begin(m_title.c_str(), &open))
    {
        const auto nodes = m_graph ? m_graph->getNodes().size() : 0;
        const auto edges = m_graph ? m_graph->getEdges().size() : 0;
        ImGui::Text("Story graph placeholder");
        ImGui::TextDisabled("Nodes: %zu  |  Edges: %zu", nodes, edges);
        ImGui::Separator();
        ImGui::TextDisabled("Double-click a script in Project to load it.");
    }
    m_visible = open;
    ImGui::End();
#else
    renderConnections();
    renderNodes();

    if (m_isConnecting)
    {
        renderPendingConnection();
    }

    renderMinimap();
#endif
}

void StoryGraphView::onResize(i32 width, i32 height)
{
    m_width = width;
    m_height = height;
}

void StoryGraphView::setVisualGraph(scripting::VisualGraph* graph)
{
    m_graph = graph;
}

void StoryGraphView::addNode(scripting::IRNodeType type, f32 x, f32 y)
{
    if (!m_graph) return;

    // Map IRNodeType to string type name for VisualGraph
    std::string typeStr;
    switch (type)
    {
        case scripting::IRNodeType::Dialogue: typeStr = "dialogue"; break;
        case scripting::IRNodeType::Choice: typeStr = "choice"; break;
        case scripting::IRNodeType::Branch: typeStr = "branch"; break;
        case scripting::IRNodeType::Goto: typeStr = "goto"; break;
        case scripting::IRNodeType::ShowCharacter: typeStr = "show_character"; break;
        case scripting::IRNodeType::HideCharacter: typeStr = "hide_character"; break;
        case scripting::IRNodeType::ShowBackground: typeStr = "show_background"; break;
        case scripting::IRNodeType::PlayMusic: typeStr = "play_music"; break;
        case scripting::IRNodeType::StopMusic: typeStr = "stop_music"; break;
        case scripting::IRNodeType::PlaySound: typeStr = "play_sound"; break;
        case scripting::IRNodeType::SceneStart: typeStr = "scene_start"; break;
        case scripting::IRNodeType::SceneEnd: typeStr = "scene_end"; break;
        default: typeStr = "unknown"; break;
    }

    // Use VisualGraph's addNode API
    m_graph->addNode(typeStr, x, y);

    if (m_onGraphModified)
    {
        m_onGraphModified();
    }
}

void StoryGraphView::deleteSelectedNodes()
{
    if (!m_graph) return;

    for (const auto& nodeId : m_selectedNodes)
    {
        // Use VisualGraph's removeNode API which handles edge cleanup
        m_graph->removeNode(nodeId);
    }

    m_selectedNodes.clear();

    if (m_onGraphModified)
    {
        m_onGraphModified();
    }
}

void StoryGraphView::duplicateSelectedNodes()
{
    if (!m_graph) return;

    std::vector<scripting::NodeId> newNodeIds;

    for (const auto& nodeId : m_selectedNodes)
    {
        // Find original node using the proper API
        const auto* node = m_graph->findNode(nodeId);
        if (node)
        {
            // Add a new node with the same type but offset position
            auto newNodeId = m_graph->addNode(node->type, node->x + 50.0f, node->y + 50.0f);

            // Copy properties
            auto* newNode = m_graph->findNode(newNodeId);
            if (newNode)
            {
                newNode->displayName = node->displayName;
                newNode->properties = node->properties;
            }

            newNodeIds.push_back(newNodeId);
        }
    }

    m_selectedNodes = std::move(newNodeIds);

    if (m_onGraphModified)
    {
        m_onGraphModified();
    }
}

void StoryGraphView::startConnection(scripting::NodeId nodeId, const std::string& portId)
{
    m_isConnecting = true;
    m_connectionStartNode = nodeId;
    m_connectionStartPort = portId;
}

void StoryGraphView::completeConnection(scripting::NodeId nodeId, const std::string& portId)
{
    if (!m_isConnecting || !m_graph) return;

    // Don't connect to self
    if (nodeId == m_connectionStartNode) {
        cancelConnection();
        return;
    }

    // Use VisualGraph's addEdge API
    m_graph->addEdge(m_connectionStartNode, m_connectionStartPort, nodeId, portId);

    cancelConnection();

    if (m_onGraphModified)
    {
        m_onGraphModified();
    }
}

void StoryGraphView::cancelConnection()
{
    m_isConnecting = false;
    m_connectionStartNode = 0;
    m_connectionStartPort.clear();
}

void StoryGraphView::setViewOffset(f32 x, f32 y)
{
    m_viewX = x;
    m_viewY = y;
}

void StoryGraphView::setViewZoom(f32 zoom)
{
    m_viewZoom = std::clamp(zoom, 0.1f, 5.0f);
}

void StoryGraphView::centerView()
{
    m_viewX = 0.0f;
    m_viewY = 0.0f;
}

void StoryGraphView::fitToContent()
{
    if (!m_graph) return;

    const auto& nodes = m_graph->getNodes();
    if (nodes.empty()) return;

    // Calculate bounding box of all nodes
    f32 minX = nodes[0].x;
    f32 maxX = nodes[0].x + nodes[0].width;
    f32 minY = nodes[0].y;
    f32 maxY = nodes[0].y + nodes[0].height;

    for (const auto& node : nodes)
    {
        minX = std::min(minX, node.x);
        maxX = std::max(maxX, node.x + node.width);
        minY = std::min(minY, node.y);
        maxY = std::max(maxY, node.y + node.height);
    }

    f32 contentWidth = maxX - minX;
    f32 contentHeight = maxY - minY;

    // Calculate zoom to fit
    f32 zoomX = static_cast<f32>(m_width) / (contentWidth + 100.0f);
    f32 zoomY = static_cast<f32>(m_height) / (contentHeight + 100.0f);
    m_viewZoom = std::min(zoomX, zoomY);
    m_viewZoom = std::clamp(m_viewZoom, 0.1f, 1.0f);

    // Center view
    m_viewX = static_cast<f32>(m_width) * 0.5f - (minX + contentWidth * 0.5f) * m_viewZoom;
    m_viewY = static_cast<f32>(m_height) * 0.5f - (minY + contentHeight * 0.5f) * m_viewZoom;
}

void StoryGraphView::selectNode(scripting::NodeId nodeId)
{
    m_selectedNodes.clear();
    m_selectedNodes.push_back(nodeId);

    if (m_onNodeSelected)
    {
        m_onNodeSelected(nodeId);
    }
}

void StoryGraphView::selectNodes(const std::vector<scripting::NodeId>& nodeIds)
{
    m_selectedNodes = nodeIds;
}

void StoryGraphView::clearSelection()
{
    m_selectedNodes.clear();
}

const std::vector<scripting::NodeId>& StoryGraphView::getSelectedNodes() const
{
    return m_selectedNodes;
}

void StoryGraphView::setOnNodeSelected(std::function<void(scripting::NodeId)> callback)
{
    m_onNodeSelected = std::move(callback);
}

void StoryGraphView::setOnGraphModified(std::function<void()> callback)
{
    m_onGraphModified = std::move(callback);
}

void StoryGraphView::renderNodes()
{
    if (!m_graph) return;

    for (const auto& node : m_graph->getNodes())
    {
        // Transform node position
        f32 screenX = node.x * m_viewZoom + m_viewX;
        f32 screenY = node.y * m_viewZoom + m_viewY;

        // Check if selected
        bool selected = std::find(m_selectedNodes.begin(), m_selectedNodes.end(), node.id)
                       != m_selectedNodes.end();

        // Render node background, title, ports
        // This would use the renderer to draw the node
        (void)screenX;
        (void)screenY;
        (void)selected;
    }
}

void StoryGraphView::renderConnections()
{
    if (!m_graph) return;

    for (const auto& edge : m_graph->getEdges())
    {
        // Find source and target nodes
        const auto* sourceNode = m_graph->findNode(edge.sourceNode);
        const auto* targetNode = m_graph->findNode(edge.targetNode);

        if (sourceNode && targetNode)
        {
            // Calculate bezier curve from source to target
            // This would use the renderer to draw the connection
        }
    }
}

void StoryGraphView::renderPendingConnection()
{
    // Render connection being created from source to mouse position
}

void StoryGraphView::renderMinimap()
{
    // Render minimap in corner showing overview of graph
}

// ============================================================================
// InspectorPanel
// ============================================================================

InspectorPanel::InspectorPanel()
    : EditorPanel("Inspector")
{
}

InspectorPanel::~InspectorPanel()
{
    if (m_inspector)
    {
        m_inspector->removeListener(this);
    }
}

void InspectorPanel::update(f64 /*deltaTime*/)
{
    // Update property values from scene
}

void InspectorPanel::render()
{
    if (!m_visible) return;

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    bool open = m_visible;
    if (ImGui::Begin(m_title.c_str(), &open))
    {
        if (!m_currentObjectId.empty() && m_inspector)
        {
            ImGui::Text("Object: %s", m_currentObjectId.c_str());
            ImGui::Separator();

            auto props = m_inspector->getProperties(m_currentObjectId);
            for (const auto& prop : props)
            {
                const std::string& label = prop.displayName.empty() ? prop.name : prop.displayName;
                ImGui::Text("%s: %s", label.c_str(), prop.value.c_str());
            }

            if (props.empty())
            {
                ImGui::TextDisabled("Нет свойств для отображения.");
            }
        }
        else if (m_currentNode)
        {
            ImGui::Text("Node id: %llu",
                        static_cast<unsigned long long>(m_currentNode->id));
            ImGui::Text("Type: %s", m_currentNode->type.c_str());
        }
        else
        {
            ImGui::TextDisabled("Ничего не выбрано.");
        }
    }
    m_visible = open;
    ImGui::End();
#else
    if (!m_currentObjectId.empty() && m_inspector)
    {
        auto props = m_inspector->getProperties(m_currentObjectId);

        // Group properties by category
        std::vector<scene::PropertyDescriptor> transform;
        std::vector<scene::PropertyDescriptor> appearance;
        std::vector<scene::PropertyDescriptor> other;

        for (const auto& prop : props)
        {
            if (prop.name == "x" || prop.name == "y" || prop.name == "rotation" ||
                prop.name == "scaleX" || prop.name == "scaleY")
            {
                transform.push_back(prop);
            }
            else if (prop.name == "alpha" || prop.name == "visible" || prop.name == "color")
            {
                appearance.push_back(prop);
            }
            else
            {
                other.push_back(prop);
            }
        }

        renderPropertyGroup("Transform", transform);
        renderPropertyGroup("Appearance", appearance);
        renderPropertyGroup("Properties", other);
    }
    else if (m_currentNode)
    {
        // Render node properties
    }
    else
    {
        // Show "Nothing selected" message
    }
#endif
}

void InspectorPanel::onResize(i32 width, i32 height)
{
    m_width = width;
    m_height = height;
}

void InspectorPanel::setInspectorAPI(scene::SceneInspectorAPI* inspector)
{
    if (m_inspector)
    {
        m_inspector->removeListener(this);
    }
    m_inspector = inspector;
    if (m_inspector)
    {
        m_inspector->addListener(this);
    }
}

void InspectorPanel::inspectObject(const std::string& objectId)
{
    m_currentObjectId = objectId;
    m_currentNode = nullptr;
}

void InspectorPanel::inspectNode(const scripting::VisualGraphNode* node)
{
    m_currentNode = node;
    m_currentObjectId.clear();
}

void InspectorPanel::clearInspection()
{
    m_currentObjectId.clear();
    m_currentNode = nullptr;
}

void InspectorPanel::onSelectionChanged(const std::vector<std::string>& selectedIds)
{
    if (selectedIds.empty())
    {
        clearInspection();
    }
    else
    {
        // Inspect first selected object
        inspectObject(selectedIds[0]);
    }
}

void InspectorPanel::onSceneModified()
{
    // Refresh property values
}

void InspectorPanel::onUndoStackChanged(bool /*canUndo*/, bool /*canRedo*/)
{
    // Nothing to do here
}

void InspectorPanel::renderPropertyGroup(const std::string& /*groupName*/,
                                          const std::vector<scene::PropertyDescriptor>& properties)
{
    for (const auto& prop : properties)
    {
        renderProperty(prop);
    }
}

void InspectorPanel::renderProperty(const scene::PropertyDescriptor& /*prop*/)
{
    // Render property editor based on type
    // This would use the UI framework to create appropriate widgets
}

// ============================================================================
// AssetBrowser
// ============================================================================

AssetBrowser::AssetBrowser()
    : EditorPanel("Assets")
{
}

void AssetBrowser::update(f64 /*deltaTime*/)
{
    // Check for file changes
}

void AssetBrowser::render()
{
    if (!m_visible) return;

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    bool open = m_visible;
    if (ImGui::Begin(m_title.c_str(), &open))
    {
        if (m_assetsPath.empty())
        {
            ImGui::TextDisabled("Assets path not set. Open a project first.");
        }
        else
        {
            ImGui::Text("Folder: %s", m_currentFolder.c_str());
            ImGui::Separator();

            if (m_assets.empty())
            {
                ImGui::TextDisabled("Папка пуста.");
            }

            for (auto& asset : m_assets)
            {
                const bool isDir = asset.type == AssetEntry::Type::Folder;
                const std::string label = std::string(isDir ? "[dir] " : "[asset] ") + asset.name;

                if (ImGui::Selectable(label.c_str(), asset.selected, ImGuiSelectableFlags_AllowDoubleClick))
                {
                    for (auto& other : m_assets) other.selected = false;
                    asset.selected = true;

                    if (m_onAssetSelected) m_onAssetSelected(asset);

                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        if (isDir)
                        {
                            m_currentFolder = asset.path;
                            refresh();
                        }
                        else if (m_onAssetDoubleClicked)
                        {
                            m_onAssetDoubleClicked(asset);
                        }
                    }
                }
            }
        }
    }
    m_visible = open;
    ImGui::End();
#endif
}

void AssetBrowser::onResize(i32 width, i32 height)
{
    m_width = width;
    m_height = height;
}

void AssetBrowser::setAssetsPath(const std::string& path)
{
    m_assetsPath = path;
    m_currentFolder = path;
    refresh();
}

void AssetBrowser::refresh()
{
    scanAssets();
}

void AssetBrowser::setViewMode(bool gridView)
{
    m_gridView = gridView;
}

void AssetBrowser::setThumbnailSize(i32 size)
{
    m_thumbnailSize = size;
}

void AssetBrowser::setOnAssetSelected(std::function<void(const AssetEntry&)> callback)
{
    m_onAssetSelected = std::move(callback);
}

void AssetBrowser::setOnAssetDoubleClicked(std::function<void(const AssetEntry&)> callback)
{
    m_onAssetDoubleClicked = std::move(callback);
}

Result<void> AssetBrowser::importAsset(const std::string& sourcePath)
{
    try
    {
        fs::path source(sourcePath);
        fs::path dest = fs::path(m_currentFolder) / source.filename();

        fs::copy(source, dest, fs::copy_options::overwrite_existing);
        refresh();

        return Result<void>::ok();
    }
    catch (const fs::filesystem_error& e)
    {
        return Result<void>::error("Failed to import asset: " + std::string(e.what()));
    }
}

Result<void> AssetBrowser::deleteAsset(const std::string& assetPath)
{
    try
    {
        fs::remove_all(assetPath);
        refresh();

        return Result<void>::ok();
    }
    catch (const fs::filesystem_error& e)
    {
        return Result<void>::error("Failed to delete asset: " + std::string(e.what()));
    }
}

Result<void> AssetBrowser::renameAsset(const std::string& oldPath, const std::string& newPath)
{
    try
    {
        fs::rename(oldPath, newPath);
        refresh();

        return Result<void>::ok();
    }
    catch (const fs::filesystem_error& e)
    {
        return Result<void>::error("Failed to rename asset: " + std::string(e.what()));
    }
}

Result<void> AssetBrowser::createFolder(const std::string& folderName)
{
    try
    {
        fs::path newFolder = fs::path(m_currentFolder) / folderName;
        fs::create_directory(newFolder);
        refresh();

        return Result<void>::ok();
    }
    catch (const fs::filesystem_error& e)
    {
        return Result<void>::error("Failed to create folder: " + std::string(e.what()));
    }
}

void AssetBrowser::scanAssets()
{
    m_assets.clear();
    m_breadcrumbs.clear();

    // Build breadcrumbs
    fs::path current(m_currentFolder);
    fs::path assets(m_assetsPath);

    auto rel = fs::relative(current, assets);
    m_breadcrumbs.push_back("Assets");

    for (const auto& part : rel)
    {
        if (part != ".")
        {
            m_breadcrumbs.push_back(part.string());
        }
    }

    // Scan directory
    try
    {
        for (const auto& entry : fs::directory_iterator(m_currentFolder))
        {
            AssetEntry asset;
            asset.name = entry.path().filename().string();
            asset.path = entry.path().string();
            asset.relativePath = fs::relative(entry.path(), m_assetsPath).string();

            if (entry.is_directory())
            {
                asset.type = AssetEntry::Type::Folder;
            }
            else
            {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                    asset.type = AssetEntry::Type::Image;
                else if (ext == ".mp3" || ext == ".wav" || ext == ".ogg")
                    asset.type = AssetEntry::Type::Audio;
                else if (ext == ".nms" || ext == ".nm")
                    asset.type = AssetEntry::Type::Script;
                else if (ext == ".ttf" || ext == ".otf")
                    asset.type = AssetEntry::Type::Font;
                else
                    asset.type = AssetEntry::Type::Unknown;

                asset.size = entry.file_size();
            }

            m_assets.push_back(std::move(asset));
        }

        // Sort
        std::sort(m_assets.begin(), m_assets.end(),
            [](const AssetEntry& a, const AssetEntry& b)
            {
                if (a.type == AssetEntry::Type::Folder && b.type != AssetEntry::Type::Folder)
                    return true;
                if (a.type != AssetEntry::Type::Folder && b.type == AssetEntry::Type::Folder)
                    return false;
                return a.name < b.name;
            });
    }
    catch (const fs::filesystem_error&)
    {
        // Handle error
    }
}

void AssetBrowser::generateThumbnail(AssetEntry& /*entry*/)
{
    // Generate thumbnail for images
    // This would use the renderer to create thumbnails
}

// ============================================================================
// PreviewWindow
// ============================================================================

PreviewWindow::PreviewWindow()
    : EditorPanel("Preview")
{
}

void PreviewWindow::update(f64 deltaTime)
{
    if (m_running && !m_paused)
    {
        m_previewTime += deltaTime;

        // Update scene in preview mode
        if (m_sceneGraph)
        {
            m_sceneGraph->update(deltaTime);
        }
    }
}

void PreviewWindow::render()
{
    if (!m_visible) return;

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    bool open = m_visible;
    if (ImGui::Begin(m_title.c_str(), &open))
    {
        ImGui::Text("Preview status: %s", m_running ? (m_paused ? "Paused" : "Running") : "Stopped");
        ImGui::TextDisabled("Use Play menu to start/stop preview.");
    }
    m_visible = open;
    ImGui::End();
#else
    // Render preview
    // Note: Actual rendering requires a renderer backend.
    // This is a stub for the preview window - rendering will be
    // implemented when a concrete renderer is attached.
    if (m_sceneGraph && m_running)
    {
        // Scene rendering is delegated to the preview backend when attached.
        // The sceneGraph->render() call requires an active renderer context.
    }
#endif
}

void PreviewWindow::onResize(i32 width, i32 height)
{
    m_width = width;
    m_height = height;
}

void PreviewWindow::startPreview()
{
    if (m_sceneGraph)
    {
        m_savedState = m_sceneGraph->saveState();
    }

    m_running = true;
    m_paused = false;
    m_previewTime = 0.0;
}

void PreviewWindow::stopPreview()
{
    m_running = false;
    m_paused = false;

    if (m_sceneGraph)
    {
        m_sceneGraph->loadState(m_savedState);
    }
}

void PreviewWindow::pausePreview()
{
    m_paused = true;
}

void PreviewWindow::resumePreview()
{
    m_paused = false;
}

void PreviewWindow::stepFrame()
{
    if (m_running && m_paused)
    {
        f64 frameTime = 1.0 / 60.0;
        m_previewTime += frameTime;

        if (m_sceneGraph)
        {
            m_sceneGraph->update(frameTime);
        }
    }
}

void PreviewWindow::setSceneGraph(scene::SceneGraph* sceneGraph)
{
    m_sceneGraph = sceneGraph;
}

// ============================================================================
// BuildManager
// ============================================================================

BuildManager::BuildManager()
{
}

void BuildManager::setProjectPath(const std::string& projectPath)
{
    m_projectPath = projectPath;
}

Result<void> BuildManager::startBuild(const BuildConfig& config)
{
    if (m_building)
    {
        return Result<void>::error("Build already in progress");
    }

    m_building = true;
    m_cancelRequested = false;
    m_currentConfig = config;
    m_progress = BuildProgress();

    // Validate
    m_progress.currentTask = "Validating project...";
    m_progress.progress = 0.0f;
    if (m_onBuildProgress) m_onBuildProgress(m_progress);

    auto result = validateProject();
    if (!result.isOk())
    {
        m_progress.errors.push_back(result.error());
        m_progress.completed = true;
        m_progress.success = false;
        m_building = false;
        if (m_onBuildComplete) m_onBuildComplete(false);
        return result;
    }

    if (m_cancelRequested) {
        m_building = false;
        return Result<void>::error("Build cancelled");
    }

    // Compile scripts
    m_progress.currentTask = "Compiling scripts...";
    m_progress.progress = 0.25f;
    if (m_onBuildProgress) m_onBuildProgress(m_progress);

    result = compileScripts();
    if (!result.isOk())
    {
        m_progress.errors.push_back(result.error());
        m_progress.completed = true;
        m_progress.success = false;
        m_building = false;
        if (m_onBuildComplete) m_onBuildComplete(false);
        return result;
    }

    if (m_cancelRequested) {
        m_building = false;
        return Result<void>::error("Build cancelled");
    }

    // Pack assets
    m_progress.currentTask = "Packing assets...";
    m_progress.progress = 0.5f;
    if (m_onBuildProgress) m_onBuildProgress(m_progress);

    result = packAssets();
    if (!result.isOk())
    {
        m_progress.errors.push_back(result.error());
        m_progress.completed = true;
        m_progress.success = false;
        m_building = false;
        if (m_onBuildComplete) m_onBuildComplete(false);
        return result;
    }

    if (m_cancelRequested) {
        m_building = false;
        return Result<void>::error("Build cancelled");
    }

    // Generate executable
    m_progress.currentTask = "Generating executable...";
    m_progress.progress = 0.75f;
    if (m_onBuildProgress) m_onBuildProgress(m_progress);

    result = generateExecutable();
    if (!result.isOk())
    {
        m_progress.errors.push_back(result.error());
        m_progress.completed = true;
        m_progress.success = false;
        m_building = false;
        if (m_onBuildComplete) m_onBuildComplete(false);
        return result;
    }

    // Complete
    m_progress.currentTask = "Build complete";
    m_progress.progress = 1.0f;
    m_progress.completed = true;
    m_progress.success = true;
    m_building = false;

    if (m_onBuildProgress) m_onBuildProgress(m_progress);
    if (m_onBuildComplete) m_onBuildComplete(true);

    return Result<void>::ok();
}

void BuildManager::cancelBuild()
{
    m_cancelRequested = true;
}

void BuildManager::setOnBuildComplete(std::function<void(bool)> callback)
{
    m_onBuildComplete = std::move(callback);
}

void BuildManager::setOnBuildProgress(std::function<void(const BuildProgress&)> callback)
{
    m_onBuildProgress = std::move(callback);
}

Result<void> BuildManager::validateProject()
{
    // Validate project structure
    fs::path projectPath(m_projectPath);

    if (!fs::exists(projectPath))
    {
        return Result<void>::error("Project path does not exist");
    }

    // Check for required files
    fs::path projectFile = projectPath / "project.json";
    if (!fs::exists(projectFile))
    {
        return Result<void>::error("project.json not found");
    }

    return Result<void>::ok();
}

Result<void> BuildManager::compileScripts()
{
    // Find and compile all .nms scripts
    fs::path scriptsPath = fs::path(m_projectPath) / "scripts";

    if (!fs::exists(scriptsPath))
    {
        // No scripts directory is OK
        return Result<void>::ok();
    }

    for (const auto& entry : fs::recursive_directory_iterator(scriptsPath))
    {
        if (entry.is_regular_file())
        {
            std::string ext = entry.path().extension().string();
            if (ext == ".nms" || ext == ".nm")
            {
                // Compile script
                // This would use the compiler to compile each script
            }
        }
    }

    return Result<void>::ok();
}

Result<void> BuildManager::packAssets()
{
    // Pack all assets into .nmpack files
    fs::path assetsPath = fs::path(m_projectPath) / "assets";
    fs::path outputPath = fs::path(m_currentConfig.outputPath) / "data";

    if (!fs::exists(assetsPath))
    {
        return Result<void>::error("Assets directory not found");
    }

    try
    {
        fs::create_directories(outputPath);

        // For now, just copy assets
        // In production, would pack into encrypted archives
        for (const auto& entry : fs::recursive_directory_iterator(assetsPath))
        {
            if (entry.is_regular_file())
            {
                auto relativePath = fs::relative(entry.path(), assetsPath);
                auto destPath = outputPath / relativePath;

                fs::create_directories(destPath.parent_path());
                fs::copy(entry.path(), destPath, fs::copy_options::overwrite_existing);
            }
        }
    }
    catch (const fs::filesystem_error& e)
    {
        return Result<void>::error("Failed to pack assets: " + std::string(e.what()));
    }

    return Result<void>::ok();
}

Result<void> BuildManager::generateExecutable()
{
    // Generate final executable
    // This would link the runtime with compiled scripts and packed assets
    // For now, this is a placeholder

    try
    {
        fs::path outputPath(m_currentConfig.outputPath);
        fs::create_directories(outputPath);

        // Create placeholder executable info
        fs::path infoFile = outputPath / "build_info.txt";
        // Would write build info here

    }
    catch (const fs::filesystem_error& e)
    {
        return Result<void>::error("Failed to generate executable: " + std::string(e.what()));
    }

    return Result<void>::ok();
}

// ============================================================================
// EditorApp
// ============================================================================

EditorApp::EditorApp()
{
}

EditorApp::~EditorApp()
{
    if (m_initialized)
    {
        shutdown();
    }
}

Result<void> EditorApp::initialize(const EditorConfig& config)
{
    m_config = config;

    // Initialize SDL2/ImGui backend
    m_backend = std::make_unique<SDLImGuiBackend>();
    EditorWindowConfig windowConfig;
    windowConfig.title = config.windowTitle;
    windowConfig.width = config.windowWidth;
    windowConfig.height = config.windowHeight;
    windowConfig.maximized = config.maximized;
    windowConfig.uiScale = config.uiScale;
    windowConfig.darkTheme = (config.theme == "dark");

    auto backendResult = m_backend->initialize(windowConfig);
    if (!backendResult.isOk())
    {
        return Result<void>::error("Failed to initialize GUI backend: " + backendResult.error());
    }

    // Set up file drop callback
    m_backend->setFileDropCallback([this](const std::string& path) {
        handleFileDropped(path);
    });

    // Create core systems
    m_sceneGraph = std::make_unique<scene::SceneGraph>();
    // SceneGraph is ready to use after construction

    m_inspectorAPI = std::make_unique<scene::SceneInspectorAPI>(m_sceneGraph.get());

    m_visualGraph = std::make_unique<scripting::VisualGraph>();
    m_converter = std::make_unique<scripting::RoundTripConverter>();

    m_buildManager = std::make_unique<BuildManager>();

    m_uiManager = std::make_unique<ui::UIManager>();
    // UIManager is ready to use after construction

    // Create panels
    setupPanels();
    setupMenuBar();
    setupToolbar();
    setupShortcuts();

    m_initialized = true;

    // Load configuration
    loadConfig();

    // Load recent projects
    loadRecentProjects();

    return Result<void>::ok();
}

void EditorApp::run()
{
    m_running = true;

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_running && !m_backend->shouldClose())
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        f64 deltaTime = std::chrono::duration<f64>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Begin new frame
        if (!m_backend->beginFrame())
        {
            break;
        }

        // Begin main dockspace
        m_backend->beginDockspace();

        // Render main menu bar
        renderMainMenuBar();

        // Process input (keyboard shortcuts, etc.)
        processInput();

        // Update panels
        update(deltaTime);

        // Render all panels
        render();

        // End dockspace
        m_backend->endDockspace();

        // End frame and swap buffers
        m_backend->endFrame();

        // Auto-save check
        if (m_config.autoSave && m_projectLoaded && m_projectModified)
        {
            m_autoSaveTimer += deltaTime;
            if (m_autoSaveTimer >= m_config.autoSaveIntervalMinutes * 60.0)
            {
                saveProject();
                m_autoSaveTimer = 0.0;
            }
        }
    }
}

void EditorApp::shutdown()
{
    saveConfig();

    m_projectBrowser.reset();
    m_sceneView.reset();
    m_storyGraphView.reset();
    m_inspectorPanel.reset();
    m_assetBrowser.reset();
    m_previewWindow.reset();

    m_uiManager.reset();
    m_buildManager.reset();
    m_converter.reset();
    m_visualGraph.reset();
    m_inspectorAPI.reset();
    m_sceneGraph.reset();

    // Shutdown backend last
    if (m_backend)
    {
        m_backend->shutdown();
        m_backend.reset();
    }

    m_initialized = false;
}

Result<void> EditorApp::newProject(const std::string& path, const std::string& name)
{
    try
    {
        fs::path projectPath(path);
        fs::create_directories(projectPath);

        // Create project structure
        fs::create_directories(projectPath / "assets" / "images");
        fs::create_directories(projectPath / "assets" / "audio");
        fs::create_directories(projectPath / "assets" / "fonts");
        fs::create_directories(projectPath / "scripts");
        fs::create_directories(projectPath / "scenes");
        fs::create_directories(projectPath / "saves");

        // Create project.json
        m_projectInfo.name = name;
        m_projectInfo.path = path;
        m_projectInfo.version = "1.0.0";
        m_projectInfo.createdTimestamp = static_cast<u64>(std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        m_projectInfo.modifiedTimestamp = m_projectInfo.createdTimestamp;

        // Save project file
        // Would serialize m_projectInfo to JSON

        m_projectLoaded = true;
        m_projectModified = false;

        // Update panels
        m_projectBrowser->setRootPath(path);
        m_assetBrowser->setAssetsPath((fs::path(path) / "assets").string());
        m_buildManager->setProjectPath(path);

        return Result<void>::ok();
    }
    catch (const fs::filesystem_error& e)
    {
        return Result<void>::error("Failed to create project: " + std::string(e.what()));
    }
}

Result<void> EditorApp::openProject(const std::string& path)
{
    try
    {
        fs::path projectPath(path);
        fs::path projectFile = projectPath / "project.json";

        if (!fs::exists(projectFile))
        {
            return Result<void>::error("Project file not found");
        }

        // Load project.json
        // Would deserialize m_projectInfo from JSON

        m_projectInfo.path = path;
        m_projectLoaded = true;
        m_projectModified = false;

        // Update panels
        m_projectBrowser->setRootPath(path);
        m_assetBrowser->setAssetsPath((fs::path(path) / "assets").string());
        m_buildManager->setProjectPath(path);

        // Add to recent projects
        m_config.lastProjectPath = path;

        return Result<void>::ok();
    }
    catch (const std::exception& e)
    {
        return Result<void>::error("Failed to open project: " + std::string(e.what()));
    }
}

Result<void> EditorApp::saveProject()
{
    if (!m_projectLoaded)
    {
        return Result<void>::error("No project loaded");
    }

    // Update timestamp
    m_projectInfo.modifiedTimestamp = static_cast<u64>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());

    // Save project.json
    // Would serialize m_projectInfo to JSON

    m_projectModified = false;

    return Result<void>::ok();
}

Result<void> EditorApp::closeProject()
{
    if (!m_projectLoaded)
    {
        return Result<void>::ok();
    }

    if (m_projectModified)
    {
        // Prompt to save changes
        // For now, just save
        auto result = saveProject();
        if (!result.isOk())
        {
            return result;
        }
    }

    m_projectLoaded = false;
    m_projectModified = false;
    m_projectInfo = ProjectInfo();

    // Clear panels
    m_projectBrowser->setRootPath("");
    m_assetBrowser->setAssetsPath("");
    m_sceneGraph->clear();
    // Reset visual graph by creating a new one
    m_visualGraph = std::make_unique<scripting::VisualGraph>();

    return Result<void>::ok();
}

Result<void> EditorApp::newScene()
{
    m_sceneGraph->clear();
    m_currentScenePath.clear();
    m_projectModified = true;

    return Result<void>::ok();
}

Result<void> EditorApp::openScene(const std::string& path)
{
    // Load scene from file
    // Would deserialize scene state from JSON

    m_currentScenePath = path;

    return Result<void>::ok();
}

Result<void> EditorApp::saveScene()
{
    if (m_currentScenePath.empty())
    {
        return Result<void>::error("No scene path set");
    }

    return saveSceneAs(m_currentScenePath);
}

Result<void> EditorApp::saveSceneAs(const std::string& path)
{
    // Save scene to file
    // Would serialize scene state to JSON

    m_currentScenePath = path;
    m_projectModified = true;

    return Result<void>::ok();
}

Result<void> EditorApp::newScript()
{
    // Reset visual graph by creating a new one
    m_visualGraph = std::make_unique<scripting::VisualGraph>();
    m_currentScriptPath.clear();
    m_projectModified = true;

    return Result<void>::ok();
}

Result<void> EditorApp::openScript(const std::string& path)
{
    // Load script from file and convert to visual graph
    // Would use RoundTripConverter

    m_currentScriptPath = path;

    return Result<void>::ok();
}

Result<void> EditorApp::saveScript()
{
    if (m_currentScriptPath.empty())
    {
        return Result<void>::error("No script path set");
    }

    return saveScriptAs(m_currentScriptPath);
}

Result<void> EditorApp::saveScriptAs(const std::string& path)
{
    // Convert visual graph to script and save
    // Would use RoundTripConverter

    m_currentScriptPath = path;
    m_projectModified = true;

    return Result<void>::ok();
}

void EditorApp::undo()
{
    m_inspectorAPI->undo();
}

void EditorApp::redo()
{
    m_inspectorAPI->redo();
}

void EditorApp::cut()
{
    copy();
    deleteSelection();
}

void EditorApp::copy()
{
    m_inspectorAPI->copySelection();
}

void EditorApp::paste()
{
    m_inspectorAPI->paste();
}

void EditorApp::deleteSelection()
{
    showConfirmation("Are you sure you want to delete the selected items?",
        [this]() {
            m_inspectorAPI->deleteSelection();
        });
}

void EditorApp::selectAll()
{
    // Select all objects in active view
    auto layers = m_inspectorAPI->getLayers();
    m_inspectorAPI->clearSelection();

    for (const auto& layer : layers)
    {
        for (const auto& objId : layer.objectIds)
        {
            m_inspectorAPI->selectObject(objId, true);
        }
    }
}

void EditorApp::setActivePanel(const std::string& /*panelName*/)
{
    // Set focus to specific panel
}

void EditorApp::togglePanel(const std::string& panelName)
{
    if (panelName == "Project")
        m_projectBrowser->setVisible(!m_projectBrowser->isVisible());
    else if (panelName == "Scene")
        m_sceneView->setVisible(!m_sceneView->isVisible());
    else if (panelName == "Story")
        m_storyGraphView->setVisible(!m_storyGraphView->isVisible());
    else if (panelName == "Inspector")
        m_inspectorPanel->setVisible(!m_inspectorPanel->isVisible());
    else if (panelName == "Assets")
        m_assetBrowser->setVisible(!m_assetBrowser->isVisible());
    else if (panelName == "Preview")
        m_previewWindow->setVisible(!m_previewWindow->isVisible());
}

void EditorApp::resetLayout()
{
    // Reset all panels to default positions and visibility
    m_projectBrowser->setVisible(true);
    m_sceneView->setVisible(true);
    m_storyGraphView->setVisible(true);
    m_inspectorPanel->setVisible(true);
    m_assetBrowser->setVisible(true);
    m_previewWindow->setVisible(false);
}

Result<void> EditorApp::build(const BuildManager::BuildConfig& config)
{
    return m_buildManager->startBuild(config);
}

Result<void> EditorApp::quickBuild()
{
    if (!m_projectLoaded)
    {
        return Result<void>::error("No project loaded");
    }

    BuildManager::BuildConfig config;
    config.outputPath = (fs::path(m_projectInfo.path) / "build").string();
    config.debug = true;

    return build(config);
}

void EditorApp::startPreview()
{
    m_previewWindow->setVisible(true);
    m_previewWindow->startPreview();
}

void EditorApp::stopPreview()
{
    m_previewWindow->stopPreview();
}

void EditorApp::setConfig(const EditorConfig& config)
{
    m_config = config;
}

void EditorApp::saveConfig()
{
    // Save editor config to file
    // Would serialize m_config to JSON
}

void EditorApp::loadConfig()
{
    // Load editor config from file
    // Would deserialize m_config from JSON
}

void EditorApp::setupMenuBar()
{
    // Setup menu bar
    // File, Edit, View, Project, Build, Help menus
}

void EditorApp::setupToolbar()
{
    // Setup toolbar with common actions
}

void EditorApp::setupPanels()
{
    m_projectBrowser = std::make_unique<ProjectBrowser>();
    m_sceneView = std::make_unique<SceneView>();
    m_storyGraphView = std::make_unique<StoryGraphView>();
    m_inspectorPanel = std::make_unique<InspectorPanel>();
    m_assetBrowser = std::make_unique<AssetBrowser>();
    m_previewWindow = std::make_unique<PreviewWindow>();

    // Connect scene view to scene graph and inspector
    m_sceneView->setSceneGraph(m_sceneGraph.get());
    m_sceneView->setInspectorAPI(m_inspectorAPI.get());

    // Connect inspector panel
    m_inspectorPanel->setInspectorAPI(m_inspectorAPI.get());

    // Connect story graph view
    m_storyGraphView->setVisualGraph(m_visualGraph.get());

    // Connect preview window
    m_previewWindow->setSceneGraph(m_sceneGraph.get());

    // Setup callbacks
    m_projectBrowser->setOnFileDoubleClicked([this](const std::string& path)
    {
        fs::path p(path);
        std::string ext = p.extension().string();

        if (ext == ".nms" || ext == ".nm")
        {
            openScript(path);
        }
        else if (ext == ".scene" || ext == ".json")
        {
            openScene(path);
        }
    });

    m_storyGraphView->setOnGraphModified([this]()
    {
        m_projectModified = true;
    });
}

void EditorApp::setupShortcuts()
{
    // Setup keyboard shortcuts
    // Ctrl+N, Ctrl+O, Ctrl+S, Ctrl+Z, Ctrl+Y, etc.
}

void EditorApp::processInput()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Get ImGui IO
    ImGuiIO& io = ImGui::GetIO();

    // Don't process shortcuts when typing in text fields
    if (io.WantTextInput) return;

    // Ctrl modifier
    bool ctrl = io.KeyCtrl;
    bool shift = io.KeyShift;
    bool alt = io.KeyAlt;

    // File menu shortcuts
    if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_N))
    {
        // Ctrl+Shift+N: New Project
        m_showNewProjectDialog = true;
    }
    else if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_N) && m_projectLoaded)
    {
        // Ctrl+N: New Scene
        newScene();
    }
    else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_O))
    {
        // Ctrl+O: Open Project
        m_showOpenProjectDialog = true;
    }
    else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_S) && m_projectLoaded)
    {
        // Ctrl+S: Save Project
        auto result = saveProject();
        if (!result.isOk())
        {
            showError("Failed to save project: " + result.error());
        }
    }

    // Edit menu shortcuts
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Z))
    {
        // Ctrl+Z: Undo
        undo();
    }
    else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Y))
    {
        // Ctrl+Y: Redo
        redo();
    }
    else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_X))
    {
        // Ctrl+X: Cut
        cut();
    }
    else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_C))
    {
        // Ctrl+C: Copy
        copy();
    }
    else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_V))
    {
        // Ctrl+V: Paste
        paste();
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        // Del: Delete
        deleteSelection();
    }
    else if (ctrl && ImGui::IsKeyPressed(ImGuiKey_A))
    {
        // Ctrl+A: Select All
        selectAll();
    }

    // Build menu shortcuts
    if (ImGui::IsKeyPressed(ImGuiKey_F7) && m_projectLoaded)
    {
        // F7: Quick Build
        auto result = quickBuild();
        if (!result.isOk())
        {
            showError("Build failed: " + result.error());
        }
    }

    // Play menu shortcuts
    if (ImGui::IsKeyPressed(ImGuiKey_F5) && m_projectLoaded)
    {
        // F5: Play
        bool isPlaying = m_previewWindow && m_previewWindow->isPreviewRunning();
        if (!isPlaying)
        {
            startPreview();
        }
        else if (shift)
        {
            // Shift+F5: Stop
            stopPreview();
        }
    }
#endif
}

void EditorApp::update(f64 deltaTime)
{
    // Update all panels
    if (m_projectBrowser) m_projectBrowser->update(deltaTime);
    if (m_sceneView) m_sceneView->update(deltaTime);
    if (m_storyGraphView) m_storyGraphView->update(deltaTime);
    if (m_inspectorPanel) m_inspectorPanel->update(deltaTime);
    if (m_assetBrowser) m_assetBrowser->update(deltaTime);
    if (m_previewWindow) m_previewWindow->update(deltaTime);

    // Update UI
    if (m_uiManager) m_uiManager->update(deltaTime);
}

void EditorApp::render()
{
    // Render all panels
    if (m_projectBrowser) m_projectBrowser->render();
    if (m_sceneView) m_sceneView->render();
    if (m_storyGraphView) m_storyGraphView->render();
    if (m_inspectorPanel) m_inspectorPanel->render();
    if (m_assetBrowser) m_assetBrowser->render();
    if (m_previewWindow) m_previewWindow->render();

    // Render UI overlays
    // Note: UIManager::render() requires a renderer - this is a stub for now
    // if (m_uiManager) m_uiManager->render(renderer);

    // Render dialogs
    renderNewProjectDialog();
    renderOpenProjectDialog();
    renderOpenSceneDialog();
    renderAboutDialogContent();
    renderErrorDialog();
    renderConfirmationDialog();
    renderProgressDialog();
    renderFileBrowserDialog();
}

void EditorApp::handleFileDropped(const std::string& path)
{
    // Import dropped file as asset
    if (m_assetBrowser)
    {
        m_assetBrowser->importAsset(path);
    }
}

void EditorApp::showWelcomeScreen()
{
    // Show welcome screen with recent projects
}

void EditorApp::showAboutDialog()
{
    // Show about dialog
}

void EditorApp::renderMainMenuBar()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (ImGui::BeginMenuBar())
    {
        // File menu
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Project", "Ctrl+Shift+N"))
            {
                m_showNewProjectDialog = true;
            }
            if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
            {
                m_showOpenProjectDialog = true;
            }
            if (ImGui::MenuItem("Save Project", "Ctrl+S", false, m_projectLoaded))
            {
                saveProject();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("New Scene", "Ctrl+N", false, m_projectLoaded))
            {
                newScene();
            }
            if (ImGui::MenuItem("Open Scene...", nullptr, false, m_projectLoaded))
            {
                m_showOpenSceneDialog = true;
            }
            if (ImGui::MenuItem("Save Scene", nullptr, false, m_projectLoaded))
            {
                saveScene();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                m_running = false;
            }
            ImGui::EndMenu();
        }

        // Edit menu
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z"))
            {
                undo();
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y"))
            {
                redo();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X"))
            {
                cut();
            }
            if (ImGui::MenuItem("Copy", "Ctrl+C"))
            {
                copy();
            }
            if (ImGui::MenuItem("Paste", "Ctrl+V"))
            {
                paste();
            }
            if (ImGui::MenuItem("Delete", "Del"))
            {
                deleteSelection();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Select All", "Ctrl+A"))
            {
                selectAll();
            }
            ImGui::EndMenu();
        }

        // View menu
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Scene View", nullptr, m_sceneView && m_sceneView->isVisible()))
            {
                togglePanel("Scene");
            }
            if (ImGui::MenuItem("Story Graph", nullptr, m_storyGraphView && m_storyGraphView->isVisible()))
            {
                togglePanel("Story");
            }
            if (ImGui::MenuItem("Inspector", nullptr, m_inspectorPanel && m_inspectorPanel->isVisible()))
            {
                togglePanel("Inspector");
            }
            if (ImGui::MenuItem("Asset Browser", nullptr, m_assetBrowser && m_assetBrowser->isVisible()))
            {
                togglePanel("Assets");
            }
            if (ImGui::MenuItem("Project Browser", nullptr, m_projectBrowser && m_projectBrowser->isVisible()))
            {
                togglePanel("Project");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Reset Layout"))
            {
                resetLayout();
            }
            ImGui::EndMenu();
        }

        // Build menu
        if (ImGui::BeginMenu("Build"))
        {
            if (ImGui::MenuItem("Quick Build", "F7", false, m_projectLoaded))
            {
                quickBuild();
            }
            if (ImGui::MenuItem("Build Settings...", nullptr, false, m_projectLoaded))
            {
                // Show build settings
            }
            ImGui::EndMenu();
        }

        // Play menu
        if (ImGui::BeginMenu("Play"))
        {
            bool isPlaying = m_previewWindow && m_previewWindow->isPreviewRunning();
            if (ImGui::MenuItem("Play", "F5", false, m_projectLoaded && !isPlaying))
            {
                startPreview();
            }
            if (ImGui::MenuItem("Stop", "Shift+F5", false, isPlaying))
            {
                stopPreview();
            }
            ImGui::EndMenu();
        }

        // Help menu
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Documentation"))
            {
                // Open documentation
            }
            if (ImGui::MenuItem("About NovelMind"))
            {
                m_showAboutDialog = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
#endif
}

void EditorApp::renderNewProjectDialog()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_showNewProjectDialog) return;

    ImGui::OpenPopup("New Project");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 350), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("New Project", &m_showNewProjectDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Create a new NovelMind project");
        ImGui::Separator();
        ImGui::Spacing();

        // Project template selection
        ImGui::Text("Template:");
        ImGui::SameLine();
        auto templates = getProjectTemplates();
        if (ImGui::BeginCombo("##template", templates[static_cast<size_t>(m_selectedTemplate)].c_str()))
        {
            for (size_t i = 0; i < templates.size(); ++i)
            {
                bool isSelected = (m_selectedTemplate == static_cast<int>(i));
                if (ImGui::Selectable(templates[i].c_str(), isSelected))
                {
                    m_selectedTemplate = static_cast<int>(i);
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();

        // Template description
        ImGui::TextWrapped("Template Info: %s",
            m_selectedTemplate == 0 ? "Start with a blank project structure." :
            m_selectedTemplate == 1 ? "Visual novel with character dialogue and branching story." :
            m_selectedTemplate == 2 ? "Dating simulator with relationship mechanics." :
            m_selectedTemplate == 3 ? "Adventure game with inventory and puzzles." :
            "Interactive storytelling with choices and consequences.");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Project Name:");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##projectname", m_newProjectNameBuffer, sizeof(m_newProjectNameBuffer));

        ImGui::Spacing();

        ImGui::Text("Project Path:");
        ImGui::SetNextItemWidth(-150);
        ImGui::InputText("##projectpath", m_newProjectPathBuffer, sizeof(m_newProjectPathBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Browse...", ImVec2(140, 0)))
        {
            m_fileBrowserMode = FileBrowserMode::OpenFolder;
            m_fileBrowserCallback = [this](const std::string& path) {
                strncpy(m_newProjectPathBuffer, path.c_str(), sizeof(m_newProjectPathBuffer) - 1);
                m_newProjectPathBuffer[sizeof(m_newProjectPathBuffer) - 1] = '\0';
            };
            m_fileBrowserNeedsRefresh = true;
            m_showFileBrowser = true;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            std::string name(m_newProjectNameBuffer);
            std::string path(m_newProjectPathBuffer);

            if (name.empty())
            {
                showError("Project name cannot be empty");
            }
            else if (path.empty())
            {
                showError("Project path cannot be empty");
            }
            else
            {
                // Show progress during project creation
                showProgress("Creating Project", "Setting up project structure...", 0.5f);

                auto result = newProject(path, name);

                hideProgress();

                if (!result.isOk())
                {
                    showError("Failed to create project: " + result.error());
                }
                else
                {
                    addToRecentProjects(path + "/" + name);
                    m_showNewProjectDialog = false;
                    // Clear buffers
                    m_newProjectNameBuffer[0] = '\0';
                    m_newProjectPathBuffer[0] = '\0';
                    m_selectedTemplate = 0;
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            m_showNewProjectDialog = false;
            m_selectedTemplate = 0;
        }

        ImGui::EndPopup();
    }
#endif
}

void EditorApp::renderOpenProjectDialog()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_showOpenProjectDialog) return;

    ImGui::OpenPopup("Open Project");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Open Project", &m_showOpenProjectDialog, ImGuiWindowFlags_None))
    {
        ImGui::Text("Open an existing NovelMind project");
        ImGui::Separator();
        ImGui::Spacing();

        // Recent projects list
        if (!m_recentProjects.empty())
        {
            ImGui::Text("Recent Projects:");
            ImGui::BeginChild("RecentProjects", ImVec2(0, 150), true);
            for (const auto& recentPath : m_recentProjects)
            {
                fs::path p(recentPath);
                std::string displayName = p.filename().string() + " - " + recentPath;

                if (ImGui::Selectable(displayName.c_str()))
                {
                    strncpy(m_openFilePathBuffer, recentPath.c_str(), sizeof(m_openFilePathBuffer) - 1);
                    m_openFilePathBuffer[sizeof(m_openFilePathBuffer) - 1] = '\0';
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                {
                    // Double-click to open directly
                    auto result = openProject(recentPath);
                    if (!result.isOk())
                    {
                        showError("Failed to open project: " + result.error());
                    }
                    else
                    {
                        addToRecentProjects(recentPath);
                        m_showOpenProjectDialog = false;
                        m_openFilePathBuffer[0] = '\0';
                    }
                }
            }
            ImGui::EndChild();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }

        ImGui::Text("Project Path:");
        ImGui::SetNextItemWidth(-150);
        ImGui::InputText("##openprojectpath", m_openFilePathBuffer, sizeof(m_openFilePathBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Browse...", ImVec2(140, 0)))
        {
            m_fileBrowserMode = FileBrowserMode::OpenFolder;
            m_fileBrowserCallback = [this](const std::string& path) {
                strncpy(m_openFilePathBuffer, path.c_str(), sizeof(m_openFilePathBuffer) - 1);
                m_openFilePathBuffer[sizeof(m_openFilePathBuffer) - 1] = '\0';
            };
            m_fileBrowserNeedsRefresh = true;
            m_showFileBrowser = true;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Open", ImVec2(120, 0)))
        {
            std::string path(m_openFilePathBuffer);

            if (path.empty())
            {
                showError("Project path cannot be empty");
            }
            else
            {
                showProgress("Opening Project", "Loading project data...", 0.5f);

                auto result = openProject(path);

                hideProgress();

                if (!result.isOk())
                {
                    showError("Failed to open project: " + result.error());
                }
                else
                {
                    addToRecentProjects(path);
                    m_showOpenProjectDialog = false;
                    m_openFilePathBuffer[0] = '\0';
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            m_showOpenProjectDialog = false;
        }

        ImGui::EndPopup();
    }
#endif
}

void EditorApp::renderOpenSceneDialog()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_showOpenSceneDialog) return;

    ImGui::OpenPopup("Open Scene");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Open Scene", &m_showOpenSceneDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Open a scene file");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Scene Path:");
        ImGui::SetNextItemWidth(-150);
        ImGui::InputText("##openscenepath", m_openFilePathBuffer, sizeof(m_openFilePathBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Browse...", ImVec2(140, 0)))
        {
            m_fileBrowserMode = FileBrowserMode::OpenFile;
            m_fileBrowserCallback = [this](const std::string& path) {
                strncpy(m_openFilePathBuffer, path.c_str(), sizeof(m_openFilePathBuffer) - 1);
                m_openFilePathBuffer[sizeof(m_openFilePathBuffer) - 1] = '\0';
            };
            m_fileBrowserNeedsRefresh = true;
            m_showFileBrowser = true;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Open", ImVec2(120, 0)))
        {
            std::string path(m_openFilePathBuffer);

            if (path.empty())
            {
                showError("Scene path cannot be empty");
            }
            else
            {
                auto result = openScene(path);
                if (!result.isOk())
                {
                    showError("Failed to open scene: " + result.error());
                }
                else
                {
                    m_showOpenSceneDialog = false;
                    m_openFilePathBuffer[0] = '\0';
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            m_showOpenSceneDialog = false;
        }

        ImGui::EndPopup();
    }
#endif
}

void EditorApp::renderAboutDialogContent()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_showAboutDialog) return;

    ImGui::OpenPopup("About NovelMind");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("About NovelMind", &m_showAboutDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("NovelMind Editor");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Version: 0.2.0 Alpha");
        ImGui::Text("Build: C++20");
        ImGui::Spacing();

        ImGui::TextWrapped("A professional editor and engine for creating visual novels with emphasis on convenience, stability, and resource protection.");
        ImGui::Spacing();

        ImGui::Text("Features:");
        ImGui::BulletText("Visual scene editing");
        ImGui::BulletText("Node-based story graph");
        ImGui::BulletText("Timeline editor");
        ImGui::BulletText("Asset management");
        ImGui::BulletText("Localization support");
        ImGui::BulletText("Voice-over system");
        ImGui::Spacing();

        ImGui::Separator();
        ImGui::Text("Copyright (c) 2024 NovelMind Contributors");
        ImGui::Text("Licensed under MIT License");
        ImGui::Spacing();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            m_showAboutDialog = false;
        }

        ImGui::EndPopup();
    }
#endif
}

void EditorApp::renderErrorDialog()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_showErrorDialog) return;

    ImGui::OpenPopup("Error");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Error", &m_showErrorDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped("%s", m_errorDialogMessage.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            m_showErrorDialog = false;
            m_errorDialogMessage.clear();
        }

        ImGui::EndPopup();
    }
#endif
}

void EditorApp::showError(const std::string& message)
{
    m_errorDialogMessage = message;
    m_showErrorDialog = true;
    NovelMind::core::Logger::instance().error(message);
}

void EditorApp::showConfirmation(const std::string& message, std::function<void()> onConfirm, std::function<void()> onCancel)
{
    m_confirmationDialogMessage = message;
    m_confirmationCallback = onConfirm;
    m_confirmationCancelCallback = onCancel;
    m_showConfirmationDialog = true;
}

void EditorApp::showProgress(const std::string& title, const std::string& message, float progress)
{
    m_progressDialogTitle = title;
    m_progressDialogMessage = message;
    m_progressDialogValue = progress;
    m_showProgressDialog = true;
}

void EditorApp::hideProgress()
{
    m_showProgressDialog = false;
}

void EditorApp::renderConfirmationDialog()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_showConfirmationDialog) return;

    ImGui::OpenPopup("Confirmation");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Confirmation", &m_showConfirmationDialog, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped("%s", m_confirmationDialogMessage.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Yes", ImVec2(120, 0)))
        {
            if (m_confirmationCallback)
            {
                m_confirmationCallback();
            }
            m_showConfirmationDialog = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(120, 0)))
        {
            if (m_confirmationCancelCallback)
            {
                m_confirmationCancelCallback();
            }
            m_showConfirmationDialog = false;
        }

        ImGui::EndPopup();
    }
#endif
}

void EditorApp::renderProgressDialog()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_showProgressDialog) return;

    ImGui::OpenPopup(m_progressDialogTitle.c_str());

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 150), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(m_progressDialogTitle.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::TextWrapped("%s", m_progressDialogMessage.c_str());
        ImGui::Spacing();
        ImGui::ProgressBar(m_progressDialogValue, ImVec2(-1.0f, 0.0f));
        ImGui::Spacing();

        ImGui::EndPopup();
    }
#endif
}

void EditorApp::renderFileBrowserDialog()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_showFileBrowser) return;

    const char* title = "Select Path";
    switch (m_fileBrowserMode)
    {
        case FileBrowserMode::OpenFile:
            title = "Open File";
            break;
        case FileBrowserMode::OpenFolder:
            title = "Select Folder";
            break;
        case FileBrowserMode::SaveFile:
            title = "Save File";
            break;
    }

    ImGui::OpenPopup(title);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal(title, &m_showFileBrowser, ImGuiWindowFlags_None))
    {
        // Initialize current path if needed
        if (m_fileBrowserCurrentPath.empty() || m_fileBrowserNeedsRefresh)
        {
            if (m_fileBrowserCurrentPath.empty())
            {
                m_fileBrowserCurrentPath = fs::current_path().string();
            }

            // Refresh directory entries
            m_fileBrowserEntries.clear();
            try
            {
                if (fs::exists(m_fileBrowserCurrentPath) && fs::is_directory(m_fileBrowserCurrentPath))
                {
                    // Add parent directory entry
                    m_fileBrowserEntries.push_back("..");

                    for (const auto& entry : fs::directory_iterator(m_fileBrowserCurrentPath))
                    {
                        m_fileBrowserEntries.push_back(entry.path().filename().string());
                    }

                    // Sort: directories first, then files
                    std::sort(m_fileBrowserEntries.begin() + 1, m_fileBrowserEntries.end(),
                        [this](const std::string& a, const std::string& b) {
                            fs::path pathA = fs::path(m_fileBrowserCurrentPath) / a;
                            fs::path pathB = fs::path(m_fileBrowserCurrentPath) / b;
                            bool isDirA = fs::is_directory(pathA);
                            bool isDirB = fs::is_directory(pathB);
                            if (isDirA != isDirB) return isDirA;
                            return a < b;
                        });
                }
            }
            catch (const std::exception& e)
            {
                NovelMind::core::Logger::instance().error("Failed to read directory: " + std::string(e.what()));
            }
            m_fileBrowserNeedsRefresh = false;
        }

        // Display current path
        ImGui::Text("Current Path:");
        ImGui::SameLine();
        ImGui::TextWrapped("%s", m_fileBrowserCurrentPath.c_str());
        ImGui::Separator();

        // File/folder list
        ImGui::BeginChild("FileList", ImVec2(0, 300), true);
        for (const auto& entry : m_fileBrowserEntries)
        {
            fs::path fullPath = fs::path(m_fileBrowserCurrentPath) / entry;
            bool isDir = entry == ".." || (fs::exists(fullPath) && fs::is_directory(fullPath));

            std::string displayName = entry;
            if (isDir && entry != "..")
            {
                displayName = "[" + entry + "]";
            }

            if (ImGui::Selectable(displayName.c_str(), m_fileBrowserSelectedPath == entry))
            {
                m_fileBrowserSelectedPath = entry;

                // Double-click or clicking on .. to navigate
                if (isDir)
                {
                    if (entry == "..")
                    {
                        m_fileBrowserCurrentPath = fs::path(m_fileBrowserCurrentPath).parent_path().string();
                    }
                    else
                    {
                        m_fileBrowserCurrentPath = fullPath.string();
                    }
                    m_fileBrowserNeedsRefresh = true;
                    m_fileBrowserSelectedPath.clear();
                }
            }

            // Double-click to enter directory
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0) && isDir && entry != "..")
            {
                m_fileBrowserCurrentPath = fullPath.string();
                m_fileBrowserNeedsRefresh = true;
                m_fileBrowserSelectedPath.clear();
            }
        }
        ImGui::EndChild();

        ImGui::Separator();
        ImGui::Spacing();

        // Buttons
        if (ImGui::Button("Select", ImVec2(120, 0)))
        {
            std::string selectedPath;
            if (m_fileBrowserMode == FileBrowserMode::OpenFolder)
            {
                selectedPath = m_fileBrowserCurrentPath;
            }
            else if (!m_fileBrowserSelectedPath.empty())
            {
                selectedPath = (fs::path(m_fileBrowserCurrentPath) / m_fileBrowserSelectedPath).string();
            }

            if (!selectedPath.empty() && m_fileBrowserCallback)
            {
                m_fileBrowserCallback(selectedPath);
            }
            m_showFileBrowser = false;
            m_fileBrowserSelectedPath.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            m_showFileBrowser = false;
            m_fileBrowserSelectedPath.clear();
        }

        ImGui::EndPopup();
    }
#endif
}

void EditorApp::loadRecentProjects()
{
    m_recentProjects.clear();

    // Try to load from config file
    fs::path configPath = fs::path(m_config.lastProjectPath).parent_path() / ".novelmind" / "recent_projects.txt";

    if (fs::exists(configPath))
    {
        try
        {
            std::ifstream file(configPath);
            std::string line;
            while (std::getline(file, line) && m_recentProjects.size() < 10)
            {
                if (!line.empty() && fs::exists(line))
                {
                    m_recentProjects.push_back(line);
                }
            }
        }
        catch (const std::exception& e)
        {
            NovelMind::core::Logger::instance().error("Failed to load recent projects: " + std::string(e.what()));
        }
    }
}

void EditorApp::saveRecentProjects()
{
    try
    {
        fs::path configDir = fs::path(m_config.lastProjectPath).parent_path() / ".novelmind";
        fs::create_directories(configDir);

        fs::path configPath = configDir / "recent_projects.txt";
        std::ofstream file(configPath);

        for (const auto& project : m_recentProjects)
        {
            file << project << "\n";
        }
    }
    catch (const std::exception& e)
    {
        NovelMind::core::Logger::instance().error("Failed to save recent projects: " + std::string(e.what()));
    }
}

void EditorApp::addToRecentProjects(const std::string& path)
{
    // Remove if already exists
    auto it = std::find(m_recentProjects.begin(), m_recentProjects.end(), path);
    if (it != m_recentProjects.end())
    {
        m_recentProjects.erase(it);
    }

    // Add to front
    m_recentProjects.insert(m_recentProjects.begin(), path);

    // Keep only 10 most recent
    if (m_recentProjects.size() > 10)
    {
        m_recentProjects.resize(10);
    }

    saveRecentProjects();
}

std::vector<std::string> EditorApp::getProjectTemplates() const
{
    return {
        "Empty Project",
        "Visual Novel Template",
        "Dating Sim Template",
        "Adventure Game Template",
        "Interactive Story Template"
    };
}

} // namespace NovelMind::editor
