/**
 * @file story_graph_panel.cpp
 * @brief Story Graph Panel implementation
 */

#include "NovelMind/editor/story_graph_panel.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
#include <imgui.h>
#endif

namespace NovelMind::editor {

StoryGraphPanel::StoryGraphPanel()
    : GUIPanelBase("Story Graph")
{
    m_flags = PanelFlags::MenuBar;
}

void StoryGraphPanel::setActiveGraph(scripting::VisualGraph* graph)
{
    m_activeGraph = graph;
    EditorSelectionManager::instance().setActiveVisualGraph(graph);

    if (m_activeGraph)
    {
        frameAll();
    }
}

void StoryGraphPanel::clearActiveGraph()
{
    m_activeGraph = nullptr;
    EditorSelectionManager::instance().setActiveVisualGraph(nullptr);
    getSelection().clearSelectionOfType(SelectionType::StoryGraphNode);
}

scripting::NodeId StoryGraphPanel::createNode(scripting::IRNodeType type, f32 x, f32 y)
{
    if (!m_activeGraph)
    {
        return 0;
    }

    std::string typeStr;
    switch (type)
    {
        case scripting::IRNodeType::Dialogue: typeStr = "dialogue"; break;
        case scripting::IRNodeType::Choice: typeStr = "choice"; break;
        case scripting::IRNodeType::ChoiceOption: typeStr = "choice_option"; break;
        case scripting::IRNodeType::SetVariable: typeStr = "set_variable"; break;
        case scripting::IRNodeType::GetVariable: typeStr = "get_variable"; break;
        case scripting::IRNodeType::Goto: typeStr = "goto"; break;
        case scripting::IRNodeType::Label: typeStr = "label"; break;
        case scripting::IRNodeType::SceneStart: typeStr = "scene_start"; break;
        case scripting::IRNodeType::SceneEnd: typeStr = "scene_end"; break;
        case scripting::IRNodeType::ShowCharacter: typeStr = "show_character"; break;
        case scripting::IRNodeType::HideCharacter: typeStr = "hide_character"; break;
        case scripting::IRNodeType::ShowBackground: typeStr = "show_background"; break;
        case scripting::IRNodeType::PlayMusic: typeStr = "play_music"; break;
        case scripting::IRNodeType::StopMusic: typeStr = "stop_music"; break;
        case scripting::IRNodeType::PlaySound: typeStr = "play_sound"; break;
        case scripting::IRNodeType::Wait: typeStr = "wait"; break;
        case scripting::IRNodeType::Branch: typeStr = "branch"; break;
        case scripting::IRNodeType::Switch: typeStr = "switch"; break;
        case scripting::IRNodeType::Sequence: typeStr = "sequence"; break;
        case scripting::IRNodeType::Loop: typeStr = "loop"; break;
        case scripting::IRNodeType::Transition: typeStr = "transition"; break;
        case scripting::IRNodeType::Expression: typeStr = "expression"; break;
        case scripting::IRNodeType::FunctionCall: typeStr = "function_call"; break;
        case scripting::IRNodeType::Comment: typeStr = "comment"; break;
        case scripting::IRNodeType::Custom: typeStr = "custom"; break;
        default: typeStr = "unknown"; break;
    }

    scripting::NodeId nodeId = m_activeGraph->addNode(typeStr, x, y);

    GraphNodeAddedEvent event;
    event.nodeId = nodeId;
    event.nodeType = type;
    event.x = x;
    event.y = y;
    publishEvent(event);

    return nodeId;
}

void StoryGraphPanel::deleteNode(scripting::NodeId nodeId)
{
    if (!m_activeGraph)
    {
        return;
    }

    removeAllConnections(nodeId);
    m_activeGraph->removeNode(nodeId);
    getSelection().removeFromSelection(SelectionItem(nodeId));

    GraphNodeRemovedEvent event;
    event.nodeId = nodeId;
    publishEvent(event);
}

void StoryGraphPanel::deleteSelectedNodes()
{
    auto selectedIds = getSelection().getSelectedNodeIds();
    for (auto nodeId : selectedIds)
    {
        deleteNode(nodeId);
    }
}

void StoryGraphPanel::duplicateSelectedNodes()
{
    if (!m_activeGraph)
    {
        return;
    }

    auto selectedIds = getSelection().getSelectedNodeIds();
    if (selectedIds.empty())
    {
        return;
    }

    std::unordered_map<scripting::NodeId, scripting::NodeId> idMap;
    std::vector<scripting::NodeId> newNodeIds;

    for (auto oldId : selectedIds)
    {
        const auto* node = m_activeGraph->findNode(oldId);
        if (!node)
        {
            continue;
        }

        scripting::NodeId newId = m_activeGraph->addNode(node->type, node->x + 50.0f, node->y + 50.0f);

        for (const auto& [key, value] : node->properties)
        {
            m_activeGraph->setNodeProperty(newId, key, value);
        }

        idMap[oldId] = newId;
        newNodeIds.push_back(newId);
    }

    for (const auto& edge : m_activeGraph->getEdges())
    {
        auto fromIt = idMap.find(edge.sourceNode);
        auto toIt = idMap.find(edge.targetNode);

        if (fromIt != idMap.end() && toIt != idMap.end())
        {
            m_activeGraph->addEdge(fromIt->second, edge.sourcePort,
                                   toIt->second, edge.targetPort);
        }
    }

    getSelection().selectNodes(newNodeIds);
}

void StoryGraphPanel::copySelectedNodes()
{
    if (!m_activeGraph)
    {
        return;
    }

    auto selectedIds = getSelection().getSelectedNodeIds();
    if (selectedIds.empty())
    {
        return;
    }

    m_clipboardNodes.clear();
    m_clipboardEdges.clear();

    for (auto nodeId : selectedIds)
    {
        const auto* node = m_activeGraph->findNode(nodeId);
        if (node)
        {
            m_clipboardNodes.push_back(*node);
        }
    }

    for (const auto& edge : m_activeGraph->getEdges())
    {
        bool fromSelected = std::find(selectedIds.begin(), selectedIds.end(), edge.sourceNode) != selectedIds.end();
        bool toSelected = std::find(selectedIds.begin(), selectedIds.end(), edge.targetNode) != selectedIds.end();

        if (fromSelected && toSelected)
        {
            m_clipboardEdges.push_back(edge);
        }
    }
}

void StoryGraphPanel::pasteNodes()
{
    if (!m_activeGraph || m_clipboardNodes.empty())
    {
        return;
    }

    std::unordered_map<scripting::NodeId, scripting::NodeId> idMap;
    std::vector<scripting::NodeId> newNodeIds;

    f32 centerX = -m_viewOffsetX + m_contentWidth / (2.0f * m_zoom);
    f32 centerY = -m_viewOffsetY + m_contentHeight / (2.0f * m_zoom);

    f32 clipboardCenterX = 0.0f;
    f32 clipboardCenterY = 0.0f;
    for (const auto& node : m_clipboardNodes)
    {
        clipboardCenterX += node.x;
        clipboardCenterY += node.y;
    }
    clipboardCenterX /= static_cast<f32>(m_clipboardNodes.size());
    clipboardCenterY /= static_cast<f32>(m_clipboardNodes.size());

    for (const auto& clipNode : m_clipboardNodes)
    {
        f32 newX = clipNode.x - clipboardCenterX + centerX;
        f32 newY = clipNode.y - clipboardCenterY + centerY;

        scripting::NodeId newId = m_activeGraph->addNode(clipNode.type, newX, newY);

        for (const auto& [key, value] : clipNode.properties)
        {
            m_activeGraph->setNodeProperty(newId, key, value);
        }

        idMap[clipNode.id] = newId;
        newNodeIds.push_back(newId);
    }

    for (const auto& clipEdge : m_clipboardEdges)
    {
        auto fromIt = idMap.find(clipEdge.sourceNode);
        auto toIt = idMap.find(clipEdge.targetNode);

        if (fromIt != idMap.end() && toIt != idMap.end())
        {
            m_activeGraph->addEdge(fromIt->second, clipEdge.sourcePort,
                                   toIt->second, clipEdge.targetPort);
        }
    }

    getSelection().selectNodes(newNodeIds);
}

bool StoryGraphPanel::createConnection(scripting::NodeId fromNode, const std::string& fromPort,
                                        scripting::NodeId toNode, const std::string& toPort)
{
    if (!m_activeGraph)
    {
        return false;
    }

    for (const auto& edge : m_activeGraph->getEdges())
    {
        if (edge.sourceNode == fromNode && edge.sourcePort == fromPort &&
            edge.targetNode == toNode && edge.targetPort == toPort)
        {
            return false;
        }
    }

    if (fromNode == toNode)
    {
        return false;
    }

    m_activeGraph->addEdge(fromNode, fromPort, toNode, toPort);

    GraphConnectionAddedEvent event;
    event.fromNode = fromNode;
    event.fromPort = fromPort;
    event.toNode = toNode;
    event.toPort = toPort;
    publishEvent(event);

    return true;
}

void StoryGraphPanel::removeConnection(scripting::NodeId fromNode, const std::string& fromPort,
                                        scripting::NodeId toNode, const std::string& toPort)
{
    if (!m_activeGraph)
    {
        return;
    }

    m_activeGraph->removeEdge(fromNode, fromPort, toNode, toPort);

    GraphConnectionRemovedEvent event;
    event.fromNode = fromNode;
    event.fromPort = fromPort;
    event.toNode = toNode;
    event.toPort = toPort;
    publishEvent(event);
}

void StoryGraphPanel::removeAllConnections(scripting::NodeId nodeId)
{
    if (!m_activeGraph)
    {
        return;
    }

    std::vector<std::tuple<scripting::NodeId, std::string, scripting::NodeId, std::string>> toRemove;

    for (const auto& edge : m_activeGraph->getEdges())
    {
        if (edge.sourceNode == nodeId || edge.targetNode == nodeId)
        {
            toRemove.emplace_back(edge.sourceNode, edge.sourcePort, edge.targetNode, edge.targetPort);
        }
    }

    for (const auto& [src, srcPort, tgt, tgtPort] : toRemove)
    {
        m_activeGraph->removeEdge(src, srcPort, tgt, tgtPort);
    }
}

void StoryGraphPanel::setViewOffset(f32 x, f32 y)
{
    m_viewOffsetX = x;
    m_viewOffsetY = y;
}

void StoryGraphPanel::setZoom(f32 zoom)
{
    m_zoom = std::clamp(zoom, m_minZoom, m_maxZoom);
}

void StoryGraphPanel::frameAll()
{
    if (!m_activeGraph || m_activeGraph->getNodes().empty())
    {
        resetView();
        return;
    }

    f32 minX = std::numeric_limits<f32>::max();
    f32 minY = std::numeric_limits<f32>::max();
    f32 maxX = std::numeric_limits<f32>::lowest();
    f32 maxY = std::numeric_limits<f32>::lowest();

    for (const auto& node : m_activeGraph->getNodes())
    {
        minX = std::min(minX, node.x);
        minY = std::min(minY, node.y);
        maxX = std::max(maxX, node.x + m_nodeStyle.nodeWidth);
        maxY = std::max(maxY, node.y + calculateNodeHeight(node));
    }

    f32 boundsWidth = maxX - minX;
    f32 boundsHeight = maxY - minY;

    if (boundsWidth > 0 && boundsHeight > 0)
    {
        f32 padding = 50.0f;
        f32 zoomX = (m_contentWidth - 2 * padding) / boundsWidth;
        f32 zoomY = (m_contentHeight - 2 * padding) / boundsHeight;
        setZoom(std::min(zoomX, zoomY));

        f32 centerX = (minX + maxX) / 2.0f;
        f32 centerY = (minY + maxY) / 2.0f;
        m_viewOffsetX = -centerX + m_contentWidth / (2.0f * m_zoom);
        m_viewOffsetY = -centerY + m_contentHeight / (2.0f * m_zoom);
    }
}

void StoryGraphPanel::frameSelection()
{
    auto selectedIds = getSelection().getSelectedNodeIds();
    if (selectedIds.empty() || !m_activeGraph)
    {
        return;
    }

    f32 minX = std::numeric_limits<f32>::max();
    f32 minY = std::numeric_limits<f32>::max();
    f32 maxX = std::numeric_limits<f32>::lowest();
    f32 maxY = std::numeric_limits<f32>::lowest();

    for (auto nodeId : selectedIds)
    {
        const auto* node = m_activeGraph->findNode(nodeId);
        if (!node)
        {
            continue;
        }

        minX = std::min(minX, node->x);
        minY = std::min(minY, node->y);
        maxX = std::max(maxX, node->x + m_nodeStyle.nodeWidth);
        maxY = std::max(maxY, node->y + calculateNodeHeight(*node));
    }

    if (minX < maxX && minY < maxY)
    {
        f32 centerX = (minX + maxX) / 2.0f;
        f32 centerY = (minY + maxY) / 2.0f;
        m_viewOffsetX = -centerX + m_contentWidth / (2.0f * m_zoom);
        m_viewOffsetY = -centerY + m_contentHeight / (2.0f * m_zoom);
    }
}

void StoryGraphPanel::resetView()
{
    m_viewOffsetX = 0.0f;
    m_viewOffsetY = 0.0f;
    m_zoom = 1.0f;
}

std::vector<scripting::NodeId> StoryGraphPanel::searchNodes(const std::string& query) const
{
    std::vector<scripting::NodeId> results;

    if (!m_activeGraph || query.empty())
    {
        return results;
    }

    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    for (const auto& node : m_activeGraph->getNodes())
    {
        std::string lowerType = node.type;
        std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), ::tolower);
        if (lowerType.find(lowerQuery) != std::string::npos)
        {
            results.push_back(node.id);
            continue;
        }

        for (const auto& [key, value] : node.properties)
        {
            std::string lowerValue = value;
            std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);
            if (lowerValue.find(lowerQuery) != std::string::npos)
            {
                results.push_back(node.id);
                break;
            }
        }
    }

    return results;
}

void StoryGraphPanel::highlightSearchResults(const std::vector<scripting::NodeId>& nodeIds)
{
    m_searchResults = nodeIds;
}

void StoryGraphPanel::clearSearchHighlight()
{
    m_searchResults.clear();
}

bool StoryGraphPanel::validateGraph()
{
    m_validationErrors.clear();
    m_errorNodes.clear();

    if (!m_activeGraph)
    {
        return true;
    }

    return m_validationErrors.empty();
}

std::vector<MenuItem> StoryGraphPanel::getMenuItems() const
{
    std::vector<MenuItem> items;

    bool hasSelection = !getSelection().getSelectedNodeIds().empty();

    MenuItem editMenu;
    editMenu.label = "Edit";
    editMenu.subItems = {
        {"Undo", "Ctrl+Z", []() { }},
        {"Redo", "Ctrl+Y", []() { }},
        MenuItem::separator(),
        {"Cut", "Ctrl+X", [this]() { const_cast<StoryGraphPanel*>(this)->copySelectedNodes(); const_cast<StoryGraphPanel*>(this)->deleteSelectedNodes(); },
         [hasSelection]() { return hasSelection; }},
        {"Copy", "Ctrl+C", [this]() { const_cast<StoryGraphPanel*>(this)->copySelectedNodes(); },
         [hasSelection]() { return hasSelection; }},
        {"Paste", "Ctrl+V", [this]() { const_cast<StoryGraphPanel*>(this)->pasteNodes(); },
         [this]() { return !m_clipboardNodes.empty(); }},
        {"Delete", "Delete", [this]() { const_cast<StoryGraphPanel*>(this)->deleteSelectedNodes(); },
         [hasSelection]() { return hasSelection; }},
        MenuItem::separator(),
        {"Duplicate", "Ctrl+D", [this]() { const_cast<StoryGraphPanel*>(this)->duplicateSelectedNodes(); },
         [hasSelection]() { return hasSelection; }},
    };
    items.push_back(editMenu);

    MenuItem viewMenu;
    viewMenu.label = "View";
    viewMenu.subItems = {
        {"Frame All", "F", [this]() { const_cast<StoryGraphPanel*>(this)->frameAll(); }},
        {"Frame Selection", "Shift+F", [this]() { const_cast<StoryGraphPanel*>(this)->frameSelection(); },
         [hasSelection]() { return hasSelection; }},
        {"Reset View", "Home", [this]() { const_cast<StoryGraphPanel*>(this)->resetView(); }},
        MenuItem::separator(),
        {"Show Minimap", "", [this]() { const_cast<StoryGraphPanel*>(this)->setMinimapVisible(!m_showMinimap); },
         []() { return true; }, [this]() { return m_showMinimap; }},
    };
    items.push_back(viewMenu);

    return items;
}

std::vector<ToolbarItem> StoryGraphPanel::getToolbarItems() const
{
    std::vector<ToolbarItem> items;

    items.push_back({"+ Dialogue", "Add Dialogue Node",
                     [this]() { const_cast<StoryGraphPanel*>(this)->createNode(scripting::IRNodeType::Dialogue, 100, 100); }});
    items.push_back({"+ Choice", "Add Choice Node",
                     [this]() { const_cast<StoryGraphPanel*>(this)->createNode(scripting::IRNodeType::Choice, 100, 200); }});
    items.push_back({"+ Goto", "Add Goto Node",
                     [this]() { const_cast<StoryGraphPanel*>(this)->createNode(scripting::IRNodeType::Goto, 100, 300); }});

    items.push_back(ToolbarItem::separator());

    items.push_back({"Frame", "Frame All (F)",
                     [this]() { const_cast<StoryGraphPanel*>(this)->frameAll(); }});

    items.push_back(ToolbarItem::separator());

    items.push_back({"Validate", "Validate Graph",
                     [this]() { const_cast<StoryGraphPanel*>(this)->validateGraph(); }});

    return items;
}

std::vector<MenuItem> StoryGraphPanel::getContextMenuItems() const
{
    std::vector<MenuItem> items;

    bool hasSelection = !getSelection().getSelectedNodeIds().empty();

    MenuItem createMenu;
    createMenu.label = "Create Node";
    createMenu.subItems = {
        {"Dialogue", "", [this]() {
            const_cast<StoryGraphPanel*>(this)->createNode(scripting::IRNodeType::Dialogue,
                m_nodeCreationMenuX, m_nodeCreationMenuY);
        }},
        {"Choice", "", [this]() {
            const_cast<StoryGraphPanel*>(this)->createNode(scripting::IRNodeType::Choice,
                m_nodeCreationMenuX, m_nodeCreationMenuY);
        }},
        {"Set Variable", "", [this]() {
            const_cast<StoryGraphPanel*>(this)->createNode(scripting::IRNodeType::SetVariable,
                m_nodeCreationMenuX, m_nodeCreationMenuY);
        }},
        {"Goto", "", [this]() {
            const_cast<StoryGraphPanel*>(this)->createNode(scripting::IRNodeType::Goto,
                m_nodeCreationMenuX, m_nodeCreationMenuY);
        }},
        {"Label", "", [this]() {
            const_cast<StoryGraphPanel*>(this)->createNode(scripting::IRNodeType::Label,
                m_nodeCreationMenuX, m_nodeCreationMenuY);
        }},
    };
    items.push_back(createMenu);

    items.push_back(MenuItem::separator());

    items.push_back({"Cut", "Ctrl+X", [this]() {
        const_cast<StoryGraphPanel*>(this)->copySelectedNodes();
        const_cast<StoryGraphPanel*>(this)->deleteSelectedNodes();
    }, [hasSelection]() { return hasSelection; }});
    items.push_back({"Copy", "Ctrl+C", [this]() {
        const_cast<StoryGraphPanel*>(this)->copySelectedNodes();
    }, [hasSelection]() { return hasSelection; }});
    items.push_back({"Paste", "Ctrl+V", [this]() {
        const_cast<StoryGraphPanel*>(this)->pasteNodes();
    }, [this]() { return !m_clipboardNodes.empty(); }});
    items.push_back({"Delete", "Delete", [this]() {
        const_cast<StoryGraphPanel*>(this)->deleteSelectedNodes();
    }, [hasSelection]() { return hasSelection; }});

    items.push_back(MenuItem::separator());

    items.push_back({"Duplicate", "Ctrl+D", [this]() {
        const_cast<StoryGraphPanel*>(this)->duplicateSelectedNodes();
    }, [hasSelection]() { return hasSelection; }});

    return items;
}

void StoryGraphPanel::onInitialize()
{
    subscribeEvent<GraphNodeAddedEvent>(
        [this](const GraphNodeAddedEvent& /*event*/) {
        });

    subscribeEvent<GraphNodeRemovedEvent>(
        [this](const GraphNodeRemovedEvent& /*event*/) {
        });

    subscribeEvent<GraphConnectionAddedEvent>(
        [this](const GraphConnectionAddedEvent& /*event*/) {
        });

    subscribeEvent<GraphConnectionRemovedEvent>(
        [this](const GraphConnectionRemovedEvent& /*event*/) {
        });
}

void StoryGraphPanel::onShutdown()
{
    clearActiveGraph();
}

void StoryGraphPanel::onUpdate(f64 /*deltaTime*/)
{
    handleMouseInput();
    handleKeyboardInput();
}

void StoryGraphPanel::onRender()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    // Get content region for rendering
    ImVec2 contentMin = ImGui::GetCursorScreenPos();
    ImVec2 contentMax = ImVec2(contentMin.x + m_contentWidth, contentMin.y + m_contentHeight);
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw graph background
    drawList->AddRectFilled(contentMin, contentMax, IM_COL32(25, 25, 25, 255));

    // Render grid
    if (m_showGrid)
    {
        f32 scaledGridSize = m_gridSize * m_zoom;
        if (scaledGridSize >= 5.0f && scaledGridSize <= 500.0f)
        {
            f32 offsetX = std::fmod(m_viewOffsetX * m_zoom + m_contentWidth / 2.0f, scaledGridSize);
            f32 offsetY = std::fmod(m_viewOffsetY * m_zoom + m_contentHeight / 2.0f, scaledGridSize);

            for (f32 x = offsetX; x < m_contentWidth; x += scaledGridSize)
            {
                drawList->AddLine(
                    ImVec2(contentMin.x + x, contentMin.y),
                    ImVec2(contentMin.x + x, contentMax.y),
                    IM_COL32(40, 40, 40, 255));
            }
            for (f32 y = offsetY; y < m_contentHeight; y += scaledGridSize)
            {
                drawList->AddLine(
                    ImVec2(contentMin.x, contentMin.y + y),
                    ImVec2(contentMax.x, contentMin.y + y),
                    IM_COL32(40, 40, 40, 255));
            }
        }
    }

    // Render graph canvas
    renderGraphCanvas();

    // Placeholder text when no graph is loaded
    if (!m_activeGraph || m_activeGraph->getNodes().empty())
    {
        ImGui::SetCursorPos(ImVec2(m_contentWidth / 2 - 120, m_contentHeight / 2));
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Story Graph - No story loaded");
        ImGui::SetCursorPos(ImVec2(m_contentWidth / 2 - 130, m_contentHeight / 2 + 20));
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Right-click to add nodes");
    }

    // Show context menu
    if (ImGui::BeginPopupContextWindow("StoryGraphContext"))
    {
        if (ImGui::BeginMenu("Add Node"))
        {
            if (ImGui::MenuItem("Dialogue")) createNode(scripting::IRNodeType::Dialogue, m_lastContextMenuX, m_lastContextMenuY);
            if (ImGui::MenuItem("Choice")) createNode(scripting::IRNodeType::Choice, m_lastContextMenuX, m_lastContextMenuY);
            if (ImGui::MenuItem("Branch")) createNode(scripting::IRNodeType::Branch, m_lastContextMenuX, m_lastContextMenuY);
            ImGui::Separator();
            if (ImGui::MenuItem("Show Character")) createNode(scripting::IRNodeType::ShowCharacter, m_lastContextMenuX, m_lastContextMenuY);
            if (ImGui::MenuItem("Hide Character")) createNode(scripting::IRNodeType::HideCharacter, m_lastContextMenuX, m_lastContextMenuY);
            if (ImGui::MenuItem("Show Background")) createNode(scripting::IRNodeType::ShowBackground, m_lastContextMenuX, m_lastContextMenuY);
            ImGui::Separator();
            if (ImGui::MenuItem("Play Music")) createNode(scripting::IRNodeType::PlayMusic, m_lastContextMenuX, m_lastContextMenuY);
            if (ImGui::MenuItem("Play Sound")) createNode(scripting::IRNodeType::PlaySound, m_lastContextMenuX, m_lastContextMenuY);
            ImGui::Separator();
            if (ImGui::MenuItem("Set Variable")) createNode(scripting::IRNodeType::SetVariable, m_lastContextMenuX, m_lastContextMenuY);
            if (ImGui::MenuItem("Get Variable")) createNode(scripting::IRNodeType::GetVariable, m_lastContextMenuX, m_lastContextMenuY);
            if (ImGui::MenuItem("Comment")) createNode(scripting::IRNodeType::Comment, m_lastContextMenuX, m_lastContextMenuY);
            ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Frame All", "F")) frameAll();
        if (ImGui::MenuItem("Frame Selection", "Shift+F", false, getSelection().hasSelectionOfType(SelectionType::StoryGraphNode))) frameSelection();
        ImGui::EndPopup();
    }
    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && m_isHovered)
    {
        auto [gx, gy] = screenToGraph(ImGui::GetMousePos().x - contentMin.x, ImGui::GetMousePos().y - contentMin.y);
        m_lastContextMenuX = gx;
        m_lastContextMenuY = gy;
        ImGui::OpenPopup("StoryGraphContext");
    }
#else
    renderGraphCanvas();
#endif
}

void StoryGraphPanel::onResize(f32 /*width*/, f32 /*height*/)
{
}

void StoryGraphPanel::renderToolbar()
{
    widgets::BeginToolbar("StoryGraphToolbar");
    renderToolbarItems(getToolbarItems());
    widgets::EndToolbar();
}

void StoryGraphPanel::onSelectionChanged(SelectionType type, const std::vector<SelectionItem>& /*selection*/)
{
    if (type == SelectionType::StoryGraphNode)
    {
    }
}

void StoryGraphPanel::renderGraphCanvas()
{
    renderGrid();
    renderConnections();
    renderNodes();

    if (m_isCreatingConnection)
    {
        renderPendingConnection();
    }

    if (m_showMinimap)
    {
        renderMinimap();
    }

    if (m_showSearch)
    {
        renderSearchBar();
    }

    if (m_showNodeCreationMenu)
    {
        renderNodeCreationMenu();
    }
}

void StoryGraphPanel::renderGrid()
{
    f32 scaledGridSize = m_gridSize * m_zoom;

    if (scaledGridSize < 5.0f || scaledGridSize > 500.0f)
    {
        return;
    }

    (void)scaledGridSize;
}

void StoryGraphPanel::renderNodes()
{
    if (!m_activeGraph)
    {
        return;
    }

    for (const auto& node : m_activeGraph->getNodes())
    {
        renderNode(node);
    }
}

void StoryGraphPanel::renderConnections()
{
    if (!m_activeGraph)
    {
        return;
    }

    for (const auto& edge : m_activeGraph->getEdges())
    {
        renderConnection(edge);
    }
}

void StoryGraphPanel::renderMinimap()
{
}

void StoryGraphPanel::renderNodeCreationMenu()
{
}

void StoryGraphPanel::renderSearchBar()
{
}

void StoryGraphPanel::renderNode(const scripting::VisualGraphNode& node)
{
    f32 screenX = (node.x + m_viewOffsetX) * m_zoom;
    f32 screenY = (node.y + m_viewOffsetY) * m_zoom;
    f32 nodeWidth = m_nodeStyle.nodeWidth * m_zoom;
    f32 nodeHeight = calculateNodeHeight(node) * m_zoom;

    renderNodeHeader(node, screenX, screenY, nodeWidth);
    renderNodeBody(node, screenX, screenY + m_nodeStyle.headerHeight * m_zoom,
                   nodeWidth, nodeHeight - m_nodeStyle.headerHeight * m_zoom);
    renderNodePins(node, screenX, screenY, nodeWidth, nodeHeight);
}

void StoryGraphPanel::renderNodeHeader(const scripting::VisualGraphNode& /*node*/, f32 /*x*/, f32 /*y*/, f32 /*width*/)
{
}

void StoryGraphPanel::renderNodeBody(const scripting::VisualGraphNode& /*node*/, f32 /*x*/, f32 /*y*/, f32 /*width*/, f32 /*height*/)
{
}

void StoryGraphPanel::renderNodePins(const scripting::VisualGraphNode& /*node*/, f32 /*x*/, f32 /*y*/, f32 /*width*/, f32 /*height*/)
{
}

void StoryGraphPanel::renderConnection(const scripting::VisualGraphEdge& edge)
{
    const auto* fromNode = m_activeGraph->findNode(edge.sourceNode);
    const auto* toNode = m_activeGraph->findNode(edge.targetNode);

    if (!fromNode || !toNode)
    {
        return;
    }

    (void)fromNode;
    (void)toNode;
}

void StoryGraphPanel::renderPendingConnection()
{
}

void StoryGraphPanel::handleMouseInput()
{
    if (!m_isHovered)
    {
        return;
    }
}

void StoryGraphPanel::handleKeyboardInput()
{
    if (!m_isFocused)
    {
        return;
    }
}

void StoryGraphPanel::handleDragDrop()
{
}

std::optional<scripting::NodeId> StoryGraphPanel::hitTestNode(f32 x, f32 y) const
{
    if (!m_activeGraph)
    {
        return std::nullopt;
    }

    f32 graphX = x / m_zoom - m_viewOffsetX;
    f32 graphY = y / m_zoom - m_viewOffsetY;

    const auto& nodes = m_activeGraph->getNodes();
    for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
    {
        const auto& node = *it;
        f32 nodeWidth = m_nodeStyle.nodeWidth;
        f32 nodeHeight = calculateNodeHeight(node);

        if (graphX >= node.x && graphX <= node.x + nodeWidth &&
            graphY >= node.y && graphY <= node.y + nodeHeight)
        {
            return node.id;
        }
    }

    return std::nullopt;
}

std::optional<std::pair<scripting::NodeId, std::string>> StoryGraphPanel::hitTestPin(f32 /*x*/, f32 /*y*/, bool /*output*/) const
{
    return std::nullopt;
}

std::pair<f32, f32> StoryGraphPanel::screenToGraph(f32 screenX, f32 screenY) const
{
    f32 graphX = screenX / m_zoom - m_viewOffsetX;
    f32 graphY = screenY / m_zoom - m_viewOffsetY;
    return {graphX, graphY};
}

std::pair<f32, f32> StoryGraphPanel::graphToScreen(f32 graphX, f32 graphY) const
{
    f32 screenX = (graphX + m_viewOffsetX) * m_zoom;
    f32 screenY = (graphY + m_viewOffsetY) * m_zoom;
    return {screenX, screenY};
}

f32 StoryGraphPanel::calculateNodeHeight(const scripting::VisualGraphNode& node) const
{
    f32 height = m_nodeStyle.headerHeight;
    height += static_cast<f32>(node.properties.size()) * 20.0f;
    height = std::max(height, 60.0f);
    return height;
}

renderer::Color StoryGraphPanel::getCategoryColor(NodeCategory category) const
{
    switch (category)
    {
        case NodeCategory::Dialogue: return {82, 139, 255, 255};
        case NodeCategory::Choice:   return {255, 193, 7, 255};
        case NodeCategory::Condition: return {255, 152, 0, 255};
        case NodeCategory::Action:   return {76, 175, 80, 255};
        case NodeCategory::Variable: return {156, 39, 176, 255};
        case NodeCategory::Event:    return {0, 188, 212, 255};
        case NodeCategory::Flow:     return {255, 87, 34, 255};
        case NodeCategory::Custom:   return {158, 158, 158, 255};
        default:                     return {100, 100, 100, 255};
    }
}

NodeCategory StoryGraphPanel::getNodeCategory(scripting::IRNodeType type) const
{
    switch (type)
    {
        case scripting::IRNodeType::Dialogue:
            return NodeCategory::Dialogue;
        case scripting::IRNodeType::Choice:
        case scripting::IRNodeType::ChoiceOption:
            return NodeCategory::Choice;
        case scripting::IRNodeType::SetVariable:
        case scripting::IRNodeType::GetVariable:
            return NodeCategory::Variable;
        case scripting::IRNodeType::Goto:
        case scripting::IRNodeType::Label:
        case scripting::IRNodeType::SceneStart:
        case scripting::IRNodeType::SceneEnd:
        case scripting::IRNodeType::Sequence:
        case scripting::IRNodeType::Loop:
            return NodeCategory::Flow;
        case scripting::IRNodeType::Branch:
        case scripting::IRNodeType::Switch:
        case scripting::IRNodeType::Expression:
            return NodeCategory::Condition;
        case scripting::IRNodeType::ShowCharacter:
        case scripting::IRNodeType::HideCharacter:
        case scripting::IRNodeType::ShowBackground:
        case scripting::IRNodeType::PlayMusic:
        case scripting::IRNodeType::StopMusic:
        case scripting::IRNodeType::PlaySound:
        case scripting::IRNodeType::Wait:
        case scripting::IRNodeType::Transition:
            return NodeCategory::Action;
        case scripting::IRNodeType::FunctionCall:
        case scripting::IRNodeType::Custom:
        case scripting::IRNodeType::Comment:
        default:
            return NodeCategory::Custom;
    }
}

std::string StoryGraphPanel::getNodeTitle(const scripting::VisualGraphNode& node) const
{
    return node.type;
}

} // namespace NovelMind::editor
