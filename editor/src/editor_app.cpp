/**
 * @file editor_app.cpp
 * @brief NovelMind Editor Application implementation
 */

#include "NovelMind/editor/editor_app.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>

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

    // Render file tree
    for (const auto& entry : m_entries)
    {
        // Render entry based on type
        // This would use the UI framework to render tree items
    }
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
            asset.modifiedTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
                modTime.time_since_epoch()).count();

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

    renderGrid();
    renderObjects();
    renderSelection();
    renderGizmos();
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

    renderConnections();
    renderNodes();

    if (m_isConnecting)
    {
        renderPendingConnection();
    }

    renderMinimap();
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

    // Render breadcrumbs
    // Render assets in grid or list view
    // This would use the UI framework
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

    // Render preview
    // Note: Actual rendering requires a renderer backend.
    // This is a stub for the preview window - rendering will be
    // implemented when a concrete renderer is attached.
    if (m_sceneGraph && m_running)
    {
        // TODO: Pass actual renderer once preview backend is set up
        // m_sceneGraph->render(renderer);
    }
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

    return Result<void>::ok();
}

void EditorApp::run()
{
    m_running = true;

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_running)
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        f64 deltaTime = std::chrono::duration<f64>(currentTime - lastTime).count();
        lastTime = currentTime;

        processInput();
        update(deltaTime);
        render();

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
        m_projectInfo.createdTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
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
    m_projectInfo.modifiedTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

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
    m_inspectorAPI->deleteSelection();
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
    // Process keyboard and mouse input
    // Handle shortcuts
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

} // namespace NovelMind::editor
