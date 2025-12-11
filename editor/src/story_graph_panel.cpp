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
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_activeGraph || m_activeGraph->getNodes().empty())
    {
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentMin = ImGui::GetCursorScreenPos();

    f32 minimapWidth = 150.0f;
    f32 minimapHeight = 100.0f;
    f32 minimapX = contentMin.x + m_contentWidth - minimapWidth - 10;
    f32 minimapY = contentMin.y + 10;

    // Draw minimap background
    drawList->AddRectFilled(
        ImVec2(minimapX, minimapY),
        ImVec2(minimapX + minimapWidth, minimapY + minimapHeight),
        IM_COL32(30, 30, 30, 200));
    drawList->AddRect(
        ImVec2(minimapX, minimapY),
        ImVec2(minimapX + minimapWidth, minimapY + minimapHeight),
        IM_COL32(80, 80, 80, 255));

    // Calculate bounds
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

    f32 rangeX = maxX - minX;
    f32 rangeY = maxY - minY;
    if (rangeX < 1.0f) rangeX = 1.0f;
    if (rangeY < 1.0f) rangeY = 1.0f;

    f32 padding = 10.0f;
    f32 scaleX = (minimapWidth - 2 * padding) / rangeX;
    f32 scaleY = (minimapHeight - 2 * padding) / rangeY;
    f32 scale = std::min(scaleX, scaleY);

    // Draw nodes on minimap
    for (const auto& node : m_activeGraph->getNodes())
    {
        f32 nx = minimapX + padding + (node.x - minX) * scale;
        f32 ny = minimapY + padding + (node.y - minY) * scale;
        f32 nw = m_nodeStyle.nodeWidth * scale;
        f32 nh = calculateNodeHeight(node) * scale;

        bool isSelected = getSelection().isNodeSelected(node.id);
        ImU32 nodeColor = isSelected ? IM_COL32(0, 150, 255, 255) : IM_COL32(100, 100, 100, 255);

        drawList->AddRectFilled(ImVec2(nx, ny), ImVec2(nx + nw, ny + nh), nodeColor);
    }

    // Draw viewport indicator
    f32 viewLeft = minimapX + padding + (-m_viewOffsetX - minX) * scale;
    f32 viewTop = minimapY + padding + (-m_viewOffsetY - minY) * scale;
    f32 viewRight = viewLeft + (m_contentWidth / m_zoom) * scale;
    f32 viewBottom = viewTop + (m_contentHeight / m_zoom) * scale;

    drawList->AddRect(
        ImVec2(viewLeft, viewTop),
        ImVec2(viewRight, viewBottom),
        IM_COL32(255, 200, 0, 255), 0.0f, 0, 2.0f);
#endif
}

void StoryGraphPanel::renderNodeCreationMenu()
{
}

void StoryGraphPanel::renderSearchBar()
{
}

void StoryGraphPanel::renderNode(const scripting::VisualGraphNode& node)
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentMin = ImGui::GetCursorScreenPos();

    f32 screenX = (node.x + m_viewOffsetX) * m_zoom + contentMin.x;
    f32 screenY = (node.y + m_viewOffsetY) * m_zoom + contentMin.y;
    f32 nodeWidth = m_nodeStyle.nodeWidth * m_zoom;
    f32 nodeHeight = calculateNodeHeight(node) * m_zoom;
    f32 headerHeight = m_nodeStyle.headerHeight * m_zoom;

    bool isSelected = getSelection().isNodeSelected(node.id);
    bool isSearchResult = std::find(m_searchResults.begin(), m_searchResults.end(), node.id) != m_searchResults.end();
    bool hasError = std::find(m_errorNodes.begin(), m_errorNodes.end(), node.id) != m_errorNodes.end();

    // Get category color
    scripting::IRNodeType nodeType = scripting::IRNodeType::Custom;
    if (node.type == "dialogue") nodeType = scripting::IRNodeType::Dialogue;
    else if (node.type == "choice") nodeType = scripting::IRNodeType::Choice;
    else if (node.type == "branch") nodeType = scripting::IRNodeType::Branch;
    else if (node.type == "set_variable") nodeType = scripting::IRNodeType::SetVariable;
    else if (node.type == "get_variable") nodeType = scripting::IRNodeType::GetVariable;
    else if (node.type == "goto") nodeType = scripting::IRNodeType::Goto;
    else if (node.type == "label") nodeType = scripting::IRNodeType::Label;
    else if (node.type == "show_character") nodeType = scripting::IRNodeType::ShowCharacter;
    else if (node.type == "play_music") nodeType = scripting::IRNodeType::PlayMusic;
    else if (node.type == "comment") nodeType = scripting::IRNodeType::Comment;

    NodeCategory cat = getNodeCategory(nodeType);
    renderer::Color catColor = getCategoryColor(cat);

    // Draw node shadow
    drawList->AddRectFilled(
        ImVec2(screenX + 3, screenY + 3),
        ImVec2(screenX + nodeWidth + 3, screenY + nodeHeight + 3),
        IM_COL32(0, 0, 0, 80), m_nodeStyle.cornerRadius * m_zoom);

    // Draw node body
    drawList->AddRectFilled(
        ImVec2(screenX, screenY),
        ImVec2(screenX + nodeWidth, screenY + nodeHeight),
        IM_COL32(40, 40, 40, 240), m_nodeStyle.cornerRadius * m_zoom);

    // Draw header
    drawList->AddRectFilled(
        ImVec2(screenX, screenY),
        ImVec2(screenX + nodeWidth, screenY + headerHeight),
        IM_COL32(catColor.r, catColor.g, catColor.b, 255),
        m_nodeStyle.cornerRadius * m_zoom, ImDrawFlags_RoundCornersTop);

    // Draw node title
    std::string title = getNodeTitle(node);
    ImVec2 titleSize = ImGui::CalcTextSize(title.c_str());
    drawList->AddText(
        ImVec2(screenX + (nodeWidth - titleSize.x) / 2, screenY + (headerHeight - titleSize.y) / 2),
        IM_COL32(255, 255, 255, 255), title.c_str());

    // Draw body content (properties)
    f32 contentY = screenY + headerHeight + 5 * m_zoom;
    for (const auto& [key, value] : node.properties)
    {
        std::string propText = key + ": " + value;
        if (propText.length() > 30) propText = propText.substr(0, 27) + "...";
        drawList->AddText(
            ImVec2(screenX + 8 * m_zoom, contentY),
            IM_COL32(200, 200, 200, 255), propText.c_str());
        contentY += 18 * m_zoom;
    }

    // Draw input pin (left side)
    f32 pinY = screenY + headerHeight / 2;
    drawList->AddCircleFilled(
        ImVec2(screenX, pinY),
        m_nodeStyle.pinRadius * m_zoom,
        IM_COL32(200, 200, 200, 255));

    // Draw output pin (right side)
    drawList->AddCircleFilled(
        ImVec2(screenX + nodeWidth, pinY),
        m_nodeStyle.pinRadius * m_zoom,
        IM_COL32(200, 200, 200, 255));

    // Draw border
    ImU32 borderColor = IM_COL32(60, 60, 60, 255);
    if (isSelected) borderColor = IM_COL32(0, 150, 255, 255);
    else if (isSearchResult) borderColor = IM_COL32(255, 200, 0, 255);
    else if (hasError) borderColor = IM_COL32(255, 80, 80, 255);

    drawList->AddRect(
        ImVec2(screenX, screenY),
        ImVec2(screenX + nodeWidth, screenY + nodeHeight),
        borderColor, m_nodeStyle.cornerRadius * m_zoom, 0, isSelected ? 3.0f : 1.0f);

#else
    f32 screenX = (node.x + m_viewOffsetX) * m_zoom;
    f32 screenY = (node.y + m_viewOffsetY) * m_zoom;
    f32 nodeWidth = m_nodeStyle.nodeWidth * m_zoom;
    f32 nodeHeight = calculateNodeHeight(node) * m_zoom;
    (void)screenX; (void)screenY; (void)nodeWidth; (void)nodeHeight;
#endif
}

void StoryGraphPanel::renderNodeHeader(const scripting::VisualGraphNode& node, f32 x, f32 y, f32 width)
{
    (void)node; (void)x; (void)y; (void)width;
    // Rendering handled in renderNode
}

void StoryGraphPanel::renderNodeBody(const scripting::VisualGraphNode& node, f32 x, f32 y, f32 width, f32 height)
{
    (void)node; (void)x; (void)y; (void)width; (void)height;
    // Rendering handled in renderNode
}

void StoryGraphPanel::renderNodePins(const scripting::VisualGraphNode& node, f32 x, f32 y, f32 width, f32 height)
{
    (void)node; (void)x; (void)y; (void)width; (void)height;
    // Rendering handled in renderNode
}

void StoryGraphPanel::renderConnection(const scripting::VisualGraphEdge& edge)
{
    const auto* fromNode = m_activeGraph->findNode(edge.sourceNode);
    const auto* toNode = m_activeGraph->findNode(edge.targetNode);

    if (!fromNode || !toNode)
    {
        return;
    }

#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentMin = ImGui::GetCursorScreenPos();

    // Calculate start point (right side of source node)
    f32 fromX = (fromNode->x + m_viewOffsetX + m_nodeStyle.nodeWidth) * m_zoom + contentMin.x;
    f32 fromY = (fromNode->y + m_viewOffsetY + m_nodeStyle.headerHeight / 2) * m_zoom + contentMin.y;

    // Calculate end point (left side of target node)
    f32 toX = (toNode->x + m_viewOffsetX) * m_zoom + contentMin.x;
    f32 toY = (toNode->y + m_viewOffsetY + m_nodeStyle.headerHeight / 2) * m_zoom + contentMin.y;

    // Draw bezier curve
    f32 tangentX = std::abs(toX - fromX) * 0.5f;
    ImVec2 p1(fromX, fromY);
    ImVec2 p2(fromX + tangentX, fromY);
    ImVec2 p3(toX - tangentX, toY);
    ImVec2 p4(toX, toY);

    // Connection color
    ImU32 connectionColor = IM_COL32(150, 150, 150, 200);

    // Check if either node is selected
    if (getSelection().isNodeSelected(edge.sourceNode) ||
        getSelection().isNodeSelected(edge.targetNode))
    {
        connectionColor = IM_COL32(0, 150, 255, 255);
    }

    drawList->AddBezierCubic(p1, p2, p3, p4, connectionColor, 2.5f);

    // Draw arrow at end
    ImVec2 dir(toX - p3.x, toY - p3.y);
    f32 len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0)
    {
        dir.x /= len;
        dir.y /= len;
        f32 arrowSize = 8.0f * m_zoom;
        ImVec2 arrowP1(toX - dir.x * arrowSize - dir.y * arrowSize * 0.5f,
                       toY - dir.y * arrowSize + dir.x * arrowSize * 0.5f);
        ImVec2 arrowP2(toX - dir.x * arrowSize + dir.y * arrowSize * 0.5f,
                       toY - dir.y * arrowSize - dir.x * arrowSize * 0.5f);
        drawList->AddTriangleFilled(ImVec2(toX, toY), arrowP1, arrowP2, connectionColor);
    }
#else
    (void)fromNode;
    (void)toNode;
#endif
}

void StoryGraphPanel::renderPendingConnection()
{
#if defined(NOVELMIND_HAS_SDL2) && defined(NOVELMIND_HAS_IMGUI)
    if (!m_isCreatingConnection || !m_activeGraph)
    {
        return;
    }

    const auto* sourceNode = m_activeGraph->findNode(m_connectionSourceNode);
    if (!sourceNode)
    {
        return;
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 contentMin = ImGui::GetCursorScreenPos();

    // Start point
    f32 fromX = (sourceNode->x + m_viewOffsetX + m_nodeStyle.nodeWidth) * m_zoom + contentMin.x;
    f32 fromY = (sourceNode->y + m_viewOffsetY + m_nodeStyle.headerHeight / 2) * m_zoom + contentMin.y;

    // End point (mouse position)
    ImVec2 mousePos = ImGui::GetMousePos();

    // Draw bezier curve to mouse
    f32 tangentX = std::abs(mousePos.x - fromX) * 0.5f;
    ImVec2 p1(fromX, fromY);
    ImVec2 p2(fromX + tangentX, fromY);
    ImVec2 p3(mousePos.x - tangentX, mousePos.y);
    ImVec2 p4(mousePos.x, mousePos.y);

    drawList->AddBezierCubic(p1, p2, p3, p4, IM_COL32(100, 200, 255, 200), 2.0f);
#endif
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
