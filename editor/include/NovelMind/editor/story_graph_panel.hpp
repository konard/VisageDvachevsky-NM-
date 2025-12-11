#pragma once

/**
 * @file story_graph_panel.hpp
 * @brief Story Graph Panel for NovelMind Editor v0.2.0
 *
 * The Story Graph panel provides a node-based visual editor for:
 * - Creating and editing dialogue trees
 * - Managing story flow and branching
 * - Setting up character interactions
 * - Configuring choice menus and consequences
 * - Variable and condition management
 *
 * Features:
 * - Drag-and-drop node creation
 * - Visual connection editing
 * - Minimap navigation
 * - Search and filter
 * - Graph validation and error highlighting
 */

#include "NovelMind/editor/gui_panel_base.hpp"
#include "NovelMind/scripting/ir.hpp"
#include "NovelMind/renderer/color.hpp"
#include <vector>
#include <unordered_map>
#include <optional>

namespace NovelMind::editor {

/**
 * @brief Node category for color coding
 */
enum class NodeCategory : u8 {
    Dialogue,
    Choice,
    Condition,
    Action,
    Variable,
    Event,
    Flow,
    Custom
};

/**
 * @brief Visual settings for node rendering
 */
struct NodeVisualStyle {
    renderer::Color headerColor{60, 60, 60, 255};
    renderer::Color bodyColor{40, 40, 40, 230};
    renderer::Color borderColor{80, 80, 80, 255};
    renderer::Color selectedBorderColor{0, 122, 204, 255};
    renderer::Color errorBorderColor{244, 67, 54, 255};
    f32 borderRadius = 4.0f;
    f32 borderWidth = 1.0f;
    f32 headerHeight = 24.0f;
    f32 nodeWidth = 200.0f;
    f32 pinRadius = 6.0f;
};

/**
 * @brief Connection visual style
 */
struct ConnectionStyle {
    renderer::Color normalColor{150, 150, 150, 255};
    renderer::Color selectedColor{0, 122, 204, 255};
    renderer::Color executionColor{255, 255, 255, 255};
    renderer::Color dataColor{100, 200, 100, 255};
    f32 thickness = 2.0f;
    f32 curvature = 0.5f;
};

/**
 * @brief Pin type for visual differentiation
 */
enum class PinType : u8 {
    Execution,  // Flow control pins
    Data,       // Data transfer pins
    Event       // Event trigger pins
};

/**
 * @brief Graph interaction mode
 */
enum class GraphInteractionMode : u8 {
    Normal,
    Panning,
    BoxSelect,
    Connecting,
    DraggingNode,
    DraggingSelection
};

/**
 * @brief Story Graph Panel implementation
 */
class StoryGraphPanel : public GUIPanelBase {
public:
    StoryGraphPanel();
    ~StoryGraphPanel() override = default;

    // =========================================================================
    // Graph Management
    // =========================================================================

    /**
     * @brief Set the active graph to edit
     */
    void setActiveGraph(scripting::VisualGraph* graph);

    /**
     * @brief Get the active graph
     */
    [[nodiscard]] scripting::VisualGraph* getActiveGraph() const { return m_activeGraph; }

    /**
     * @brief Clear the active graph
     */
    void clearActiveGraph();

    // =========================================================================
    // Node Operations
    // =========================================================================

    /**
     * @brief Create a new node at position
     */
    scripting::NodeId createNode(scripting::IRNodeType type, f32 x, f32 y);

    /**
     * @brief Delete a node
     */
    void deleteNode(scripting::NodeId nodeId);

    /**
     * @brief Delete selected nodes
     */
    void deleteSelectedNodes();

    /**
     * @brief Duplicate selected nodes
     */
    void duplicateSelectedNodes();

    /**
     * @brief Copy selected nodes to clipboard
     */
    void copySelectedNodes();

    /**
     * @brief Paste nodes from clipboard
     */
    void pasteNodes();

    // =========================================================================
    // Connection Operations
    // =========================================================================

    /**
     * @brief Create a connection between nodes
     */
    bool createConnection(scripting::NodeId fromNode, const std::string& fromPort,
                         scripting::NodeId toNode, const std::string& toPort);

    /**
     * @brief Remove a connection
     */
    void removeConnection(scripting::NodeId fromNode, const std::string& fromPort,
                         scripting::NodeId toNode, const std::string& toPort);

    /**
     * @brief Remove all connections from a node
     */
    void removeAllConnections(scripting::NodeId nodeId);

    // =========================================================================
    // View State
    // =========================================================================

    /**
     * @brief Get view offset (pan)
     */
    [[nodiscard]] std::pair<f32, f32> getViewOffset() const {
        return {m_viewOffsetX, m_viewOffsetY};
    }

    /**
     * @brief Set view offset
     */
    void setViewOffset(f32 x, f32 y);

    /**
     * @brief Get zoom level
     */
    [[nodiscard]] f32 getZoom() const { return m_zoom; }

    /**
     * @brief Set zoom level
     */
    void setZoom(f32 zoom);

    /**
     * @brief Frame all nodes in view
     */
    void frameAll();

    /**
     * @brief Frame selected nodes in view
     */
    void frameSelection();

    /**
     * @brief Reset view to origin
     */
    void resetView();

    // =========================================================================
    // Minimap
    // =========================================================================

    /**
     * @brief Check if minimap is visible
     */
    [[nodiscard]] bool isMinimapVisible() const { return m_showMinimap; }

    /**
     * @brief Set minimap visibility
     */
    void setMinimapVisible(bool visible) { m_showMinimap = visible; }

    // =========================================================================
    // Search and Filter
    // =========================================================================

    /**
     * @brief Search for nodes
     */
    std::vector<scripting::NodeId> searchNodes(const std::string& query) const;

    /**
     * @brief Highlight search results
     */
    void highlightSearchResults(const std::vector<scripting::NodeId>& nodeIds);

    /**
     * @brief Clear search highlight
     */
    void clearSearchHighlight();

    // =========================================================================
    // Validation
    // =========================================================================

    /**
     * @brief Validate the graph
     */
    bool validateGraph();

    /**
     * @brief Get validation errors
     */
    [[nodiscard]] const std::vector<std::string>& getValidationErrors() const {
        return m_validationErrors;
    }

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
    void renderGraphCanvas();
    void renderGrid();
    void renderNodes();
    void renderConnections();
    void renderMinimap();
    void renderNodeCreationMenu();
    void renderSearchBar();

    // Node rendering
    void renderNode(const scripting::VisualGraphNode& node);
    void renderNodeHeader(const scripting::VisualGraphNode& node, f32 x, f32 y, f32 width);
    void renderNodeBody(const scripting::VisualGraphNode& node, f32 x, f32 y, f32 width, f32 height);
    void renderNodePins(const scripting::VisualGraphNode& node, f32 x, f32 y, f32 width, f32 height);
    void renderConnection(const scripting::VisualGraphEdge& edge);
    void renderPendingConnection();

    // Input handling
    void handleMouseInput();
    void handleKeyboardInput();
    void handleDragDrop();

    // Hit testing
    std::optional<scripting::NodeId> hitTestNode(f32 x, f32 y) const;
    std::optional<std::pair<scripting::NodeId, std::string>> hitTestPin(f32 x, f32 y, bool output) const;

    // Coordinate conversion
    std::pair<f32, f32> screenToGraph(f32 screenX, f32 screenY) const;
    std::pair<f32, f32> graphToScreen(f32 graphX, f32 graphY) const;

    // Node helpers
    f32 calculateNodeHeight(const scripting::VisualGraphNode& node) const;
    renderer::Color getCategoryColor(NodeCategory category) const;
    NodeCategory getNodeCategory(scripting::IRNodeType type) const;
    std::string getNodeTitle(const scripting::VisualGraphNode& node) const;

    // State
    scripting::VisualGraph* m_activeGraph = nullptr;

    // View state
    f32 m_viewOffsetX = 0.0f;
    f32 m_viewOffsetY = 0.0f;
    f32 m_zoom = 1.0f;
    f32 m_minZoom = 0.1f;
    f32 m_maxZoom = 4.0f;

    // Interaction state
    GraphInteractionMode m_interactionMode = GraphInteractionMode::Normal;
    f32 m_lastMouseX = 0.0f;
    f32 m_lastMouseY = 0.0f;
    f32 m_dragStartX = 0.0f;
    f32 m_dragStartY = 0.0f;

    // Box select
    f32 m_boxSelectStartX = 0.0f;
    f32 m_boxSelectStartY = 0.0f;
    f32 m_boxSelectEndX = 0.0f;
    f32 m_boxSelectEndY = 0.0f;

    // Connection creation
    bool m_isCreatingConnection = false;
    scripting::NodeId m_connectionSourceNode = 0;
    std::string m_connectionSourcePort;
    bool m_connectionIsOutput = false;
    f32 m_connectionEndX = 0.0f;
    f32 m_connectionEndY = 0.0f;

    // Minimap
    bool m_showMinimap = true;
    f32 m_minimapSize = 150.0f;

    // Search
    char m_searchBuffer[256] = {0};
    std::vector<scripting::NodeId> m_searchResults;
    bool m_showSearch = false;

    // Node creation menu
    bool m_showNodeCreationMenu = false;
    f32 m_nodeCreationMenuX = 0.0f;
    f32 m_nodeCreationMenuY = 0.0f;

    // Context menu position
    f32 m_lastContextMenuX = 0.0f;
    f32 m_lastContextMenuY = 0.0f;

    // Visual styles
    NodeVisualStyle m_nodeStyle;
    ConnectionStyle m_connectionStyle;

    // Grid
    bool m_showGrid = true;
    f32 m_gridSize = 20.0f;
    renderer::Color m_gridColor{40, 40, 40, 255};
    renderer::Color m_gridMajorColor{50, 50, 50, 255};

    // Validation
    std::vector<std::string> m_validationErrors;
    std::vector<scripting::NodeId> m_errorNodes;

    // Clipboard
    std::vector<scripting::VisualGraphNode> m_clipboardNodes;
    std::vector<scripting::VisualGraphEdge> m_clipboardEdges;
};

} // namespace NovelMind::editor
