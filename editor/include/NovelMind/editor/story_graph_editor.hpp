#pragma once

/**
 * @file story_graph_editor.hpp
 * @brief Story Graph Editor for NovelMind
 *
 * Professional node-based story editor inspired by Unreal Blueprints:
 * - Visual node editing with smooth bezier connections
 * - Real-time validation (unreachable nodes, cycles, missing scenes)
 * - Play-In-Graph integration with active node highlighting
 * - Smart edge snapping and grid alignment
 * - Comprehensive node types for visual novel authoring
 */

#include "NovelMind/core/types.hpp"
#include "NovelMind/core/result.hpp"
#include "NovelMind/scripting/ir.hpp"
#include "NovelMind/renderer/renderer.hpp"
#include "NovelMind/renderer/color.hpp"
#include "editor_app.hpp"
#include "undo_system.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace NovelMind::editor
{

// Forward declarations
class StoryGraphEditor;
class StoryNode;
class StoryConnection;

/**
 * @brief Validation issue severity
 */
enum class ValidationSeverity : u8
{
    Info,
    Warning,
    Error
};

/**
 * @brief A validation issue found in the story graph
 */
struct ValidationIssue
{
    ValidationSeverity severity;
    std::string message;
    std::string nodeId;          // Empty if graph-level issue
    std::string propertyName;    // Empty if node-level issue

    // Location for navigation
    f32 x = 0.0f;
    f32 y = 0.0f;
};

/**
 * @brief Story graph validation result
 */
struct ValidationResult
{
    bool isValid;
    std::vector<ValidationIssue> issues;

    // Statistics
    i32 totalNodes = 0;
    i32 unreachableNodes = 0;
    i32 cycleCount = 0;
    i32 missingReferences = 0;
    i32 duplicateIds = 0;

    [[nodiscard]] bool hasErrors() const;
    [[nodiscard]] bool hasWarnings() const;
    [[nodiscard]] std::vector<ValidationIssue> getErrors() const;
    [[nodiscard]] std::vector<ValidationIssue> getWarnings() const;
};

/**
 * @brief Node visual style configuration
 */
struct NodeStyle
{
    renderer::Color headerColor = {0.2f, 0.4f, 0.6f, 1.0f};
    renderer::Color bodyColor = {0.15f, 0.15f, 0.15f, 0.95f};
    renderer::Color borderColor = {0.3f, 0.3f, 0.3f, 1.0f};
    renderer::Color selectedBorderColor = {1.0f, 0.6f, 0.0f, 1.0f};
    renderer::Color textColor = {1.0f, 1.0f, 1.0f, 1.0f};
    renderer::Color errorBorderColor = {0.9f, 0.2f, 0.2f, 1.0f};
    renderer::Color warningBorderColor = {0.9f, 0.7f, 0.1f, 1.0f};
    renderer::Color activeHighlightColor = {0.2f, 0.8f, 0.4f, 1.0f};

    f32 borderRadius = 8.0f;
    f32 borderWidth = 2.0f;
    f32 headerHeight = 28.0f;
    f32 minWidth = 180.0f;
    f32 maxWidth = 400.0f;
};

/**
 * @brief Port visual style
 */
struct PortStyle
{
    renderer::Color flowPortColor = {1.0f, 1.0f, 1.0f, 1.0f};
    renderer::Color dataPortColor = {0.4f, 0.7f, 1.0f, 1.0f};
    renderer::Color connectedColor = {0.2f, 0.8f, 0.4f, 1.0f};
    renderer::Color hoveredColor = {1.0f, 0.8f, 0.0f, 1.0f};

    f32 portRadius = 6.0f;
    f32 portSpacing = 24.0f;
};

/**
 * @brief Connection visual style
 */
struct ConnectionStyle
{
    renderer::Color flowColor = {1.0f, 1.0f, 1.0f, 0.8f};
    renderer::Color dataColor = {0.4f, 0.7f, 1.0f, 0.8f};
    renderer::Color selectedColor = {1.0f, 0.6f, 0.0f, 1.0f};
    renderer::Color activeColor = {0.2f, 0.8f, 0.4f, 1.0f};

    f32 lineWidth = 3.0f;
    f32 selectedLineWidth = 4.0f;
    f32 curveTension = 0.5f;  // Bezier curve smoothness
    bool showArrows = true;
    f32 arrowSize = 10.0f;
};

/**
 * @brief Graph editor configuration
 */
struct StoryGraphConfig
{
    // Grid settings
    bool showGrid = true;
    bool snapToGrid = true;
    f32 gridSize = 20.0f;
    f32 majorGridInterval = 5.0f;
    renderer::Color gridColor = {0.2f, 0.2f, 0.2f, 1.0f};
    renderer::Color majorGridColor = {0.25f, 0.25f, 0.25f, 1.0f};

    // Zoom settings
    f32 minZoom = 0.1f;
    f32 maxZoom = 3.0f;
    f32 zoomStep = 0.1f;

    // Selection
    bool multiSelectEnabled = true;
    renderer::Color selectionBoxColor = {0.3f, 0.5f, 0.8f, 0.3f};
    renderer::Color selectionBoxBorderColor = {0.3f, 0.5f, 0.8f, 1.0f};

    // Edge snapping
    bool edgeSnapping = true;
    f32 edgeSnapDistance = 20.0f;

    // Minimap
    bool showMinimap = true;
    f32 minimapSize = 150.0f;
    f32 minimapOpacity = 0.8f;

    // Validation
    bool realTimeValidation = true;
    f32 validationDebounceMs = 500.0f;

    // Styles
    NodeStyle nodeStyle;
    PortStyle portStyle;
    ConnectionStyle connectionStyle;
};

/**
 * @brief Story node types (visual novel specific)
 */
enum class StoryNodeType : u8
{
    // Entry/Exit
    Start,              // Scene entry point
    End,                // Scene end/return

    // Dialogue
    Dialogue,           // Character dialogue
    Narration,          // Narrator text
    InnerMonologue,     // Character thought

    // Choices
    Choice,             // Player choice menu
    ConditionalChoice,  // Choice with conditions

    // Flow control
    Branch,             // If/else condition
    Switch,             // Multi-way branch
    Goto,               // Jump to label/scene
    Label,              // Jump target

    // Scene
    SceneChange,        // Transition to another scene
    SceneCall,          // Call scene and return

    // Characters
    ShowCharacter,      // Show character sprite
    HideCharacter,      // Hide character
    MoveCharacter,      // Animate character position
    ChangeExpression,   // Change character expression

    // Background
    ChangeBackground,   // Change background image
    BackgroundEffect,   // Apply effect to background

    // Audio
    PlayMusic,          // Play BGM
    StopMusic,          // Stop BGM
    PlaySound,          // Play SFX
    PlayVoice,          // Play voice line

    // Effects
    Transition,         // Screen transition
    CameraMove,         // Camera pan/zoom
    ScreenEffect,       // Shake, flash, etc.
    Wait,               // Delay execution

    // Variables
    SetVariable,        // Set variable value
    GetVariable,        // Read variable
    ModifyVariable,     // Increment/decrement

    // Advanced
    Script,             // Custom NM Script block
    Event,              // Fire custom event
    Comment,            // Documentation node (non-executable)
    Group,              // Visual grouping container

    // Markers
    Bookmark,           // Save point marker
    Achievement,        // Achievement trigger

    Custom              // User-defined node type
};

/**
 * @brief A port on a story node
 */
struct StoryPort
{
    std::string id;
    std::string displayName;
    bool isInput;
    bool isFlow;            // True for execution flow, false for data
    std::string dataType;   // For data ports: "string", "int", "bool", etc.
    bool required = false;
    bool connected = false;

    // Visual position (relative to node)
    f32 offsetY = 0.0f;
};

/**
 * @brief A node in the story graph
 */
class StoryNode
{
public:
    StoryNode(scripting::NodeId id, StoryNodeType type);
    virtual ~StoryNode() = default;

    [[nodiscard]] scripting::NodeId getId() const { return m_id; }
    [[nodiscard]] StoryNodeType getType() const { return m_type; }
    [[nodiscard]] const char* getTypeName() const;
    [[nodiscard]] const char* getTypeDisplayName() const;

    // Properties
    void setProperty(const std::string& name, const std::string& value);
    [[nodiscard]] std::optional<std::string> getProperty(const std::string& name) const;
    [[nodiscard]] const std::unordered_map<std::string, std::string>& getProperties() const { return m_properties; }

    // Common properties (convenience accessors)
    void setTitle(const std::string& title) { m_title = title; }
    [[nodiscard]] const std::string& getTitle() const { return m_title; }

    void setDescription(const std::string& desc) { m_description = desc; }
    [[nodiscard]] const std::string& getDescription() const { return m_description; }

    // Position
    void setPosition(f32 x, f32 y);
    [[nodiscard]] f32 getX() const { return m_x; }
    [[nodiscard]] f32 getY() const { return m_y; }

    void setSize(f32 width, f32 height);
    [[nodiscard]] f32 getWidth() const { return m_width; }
    [[nodiscard]] f32 getHeight() const { return m_height; }

    // Ports
    void addInputPort(const StoryPort& port);
    void addOutputPort(const StoryPort& port);
    void removePort(const std::string& portId);
    [[nodiscard]] const std::vector<StoryPort>& getInputPorts() const { return m_inputPorts; }
    [[nodiscard]] const std::vector<StoryPort>& getOutputPorts() const { return m_outputPorts; }
    [[nodiscard]] StoryPort* getPort(const std::string& portId);
    [[nodiscard]] const StoryPort* getPort(const std::string& portId) const;

    // State
    void setSelected(bool selected) { m_selected = selected; }
    [[nodiscard]] bool isSelected() const { return m_selected; }

    void setCollapsed(bool collapsed) { m_collapsed = collapsed; }
    [[nodiscard]] bool isCollapsed() const { return m_collapsed; }

    void setHasError(bool hasError) { m_hasError = hasError; }
    [[nodiscard]] bool hasError() const { return m_hasError; }

    void setHasWarning(bool hasWarning) { m_hasWarning = hasWarning; }
    [[nodiscard]] bool hasWarning() const { return m_hasWarning; }

    void setActive(bool active) { m_active = active; }
    [[nodiscard]] bool isActive() const { return m_active; }

    void setVisited(bool visited) { m_visited = visited; }
    [[nodiscard]] bool isVisited() const { return m_visited; }

    // Serialization
    [[nodiscard]] std::string toJson() const;
    static Result<std::unique_ptr<StoryNode>> fromJson(const std::string& json);

protected:
    void setupDefaultPorts();

    scripting::NodeId m_id;
    StoryNodeType m_type;
    std::string m_title;
    std::string m_description;

    f32 m_x = 0.0f;
    f32 m_y = 0.0f;
    f32 m_width = 200.0f;
    f32 m_height = 100.0f;

    std::vector<StoryPort> m_inputPorts;
    std::vector<StoryPort> m_outputPorts;
    std::unordered_map<std::string, std::string> m_properties;

    bool m_selected = false;
    bool m_collapsed = false;
    bool m_hasError = false;
    bool m_hasWarning = false;
    bool m_active = false;      // Currently executing (Play-In-Graph)
    bool m_visited = false;     // Has been executed (Play-In-Graph)
};

/**
 * @brief A connection between two story nodes
 */
class StoryConnection
{
public:
    StoryConnection(scripting::NodeId sourceNode, const std::string& sourcePort,
                    scripting::NodeId targetNode, const std::string& targetPort);

    [[nodiscard]] scripting::NodeId getSourceNode() const { return m_sourceNode; }
    [[nodiscard]] const std::string& getSourcePort() const { return m_sourcePort; }
    [[nodiscard]] scripting::NodeId getTargetNode() const { return m_targetNode; }
    [[nodiscard]] const std::string& getTargetPort() const { return m_targetPort; }

    void setSelected(bool selected) { m_selected = selected; }
    [[nodiscard]] bool isSelected() const { return m_selected; }

    void setActive(bool active) { m_active = active; }
    [[nodiscard]] bool isActive() const { return m_active; }

private:
    scripting::NodeId m_sourceNode;
    std::string m_sourcePort;
    scripting::NodeId m_targetNode;
    std::string m_targetPort;

    bool m_selected = false;
    bool m_active = false;
};

/**
 * @brief Story Graph data structure
 */
class StoryGraph
{
public:
    StoryGraph(const std::string& name = "Untitled");
    ~StoryGraph();

    // Graph identity
    void setName(const std::string& name) { m_name = name; }
    [[nodiscard]] const std::string& getName() const { return m_name; }

    // Node management
    scripting::NodeId createNode(StoryNodeType type, f32 x = 0.0f, f32 y = 0.0f);
    void removeNode(scripting::NodeId id);
    [[nodiscard]] StoryNode* getNode(scripting::NodeId id);
    [[nodiscard]] const StoryNode* getNode(scripting::NodeId id) const;
    [[nodiscard]] std::vector<StoryNode*> getNodes();
    [[nodiscard]] std::vector<const StoryNode*> getNodes() const;
    [[nodiscard]] std::vector<StoryNode*> getNodesByType(StoryNodeType type);

    // Connection management
    Result<void> connect(scripting::NodeId sourceNode, const std::string& sourcePort,
                         scripting::NodeId targetNode, const std::string& targetPort);
    void disconnect(scripting::NodeId sourceNode, const std::string& sourcePort,
                    scripting::NodeId targetNode, const std::string& targetPort);
    void disconnectAll(scripting::NodeId nodeId);
    [[nodiscard]] std::vector<StoryConnection>& getConnections() { return m_connections; }
    [[nodiscard]] const std::vector<StoryConnection>& getConnections() const { return m_connections; }
    [[nodiscard]] std::vector<StoryConnection*> getConnectionsFrom(scripting::NodeId nodeId);
    [[nodiscard]] std::vector<StoryConnection*> getConnectionsTo(scripting::NodeId nodeId);

    // Selection
    void selectNode(scripting::NodeId id, bool addToSelection = false);
    void selectNodes(const std::vector<scripting::NodeId>& ids);
    void deselectNode(scripting::NodeId id);
    void selectAll();
    void deselectAll();
    [[nodiscard]] std::vector<scripting::NodeId> getSelectedNodes() const;
    [[nodiscard]] bool isNodeSelected(scripting::NodeId id) const;

    void selectConnection(const StoryConnection& conn);
    void deselectAllConnections();
    [[nodiscard]] std::vector<StoryConnection*> getSelectedConnections();

    // Entry point
    void setStartNode(scripting::NodeId id) { m_startNodeId = id; }
    [[nodiscard]] scripting::NodeId getStartNode() const { return m_startNodeId; }

    // Variables defined in this graph
    void addVariable(const std::string& name, const std::string& type, const std::string& defaultValue);
    void removeVariable(const std::string& name);
    [[nodiscard]] bool hasVariable(const std::string& name) const;

    // Validation
    [[nodiscard]] ValidationResult validate() const;

    // Conversion to/from IR
    [[nodiscard]] std::unique_ptr<scripting::IRGraph> toIR() const;
    static Result<std::unique_ptr<StoryGraph>> fromIR(const scripting::IRGraph& ir);

    // Serialization
    [[nodiscard]] std::string toJson() const;
    static Result<std::unique_ptr<StoryGraph>> fromJson(const std::string& json);
    Result<void> save(const std::string& path) const;
    static Result<std::unique_ptr<StoryGraph>> load(const std::string& path);

private:
    void detectCycles(std::vector<ValidationIssue>& issues) const;
    void detectUnreachable(std::vector<ValidationIssue>& issues) const;
    void validateConnections(std::vector<ValidationIssue>& issues) const;
    void validateNodeProperties(std::vector<ValidationIssue>& issues) const;

    std::string m_name;
    scripting::NodeId m_nextId = 1;
    scripting::NodeId m_startNodeId = 0;

    std::unordered_map<scripting::NodeId, std::unique_ptr<StoryNode>> m_nodes;
    std::vector<StoryConnection> m_connections;

    // Variables
    struct VariableDecl
    {
        std::string type;
        std::string defaultValue;
    };
    std::unordered_map<std::string, VariableDecl> m_variables;
};

/**
 * @brief Story Graph Editor Panel
 *
 * Full-featured visual editor for story graphs:
 * - Drag-and-drop node creation
 * - Smooth bezier curve connections
 * - Real-time validation with error highlighting
 * - Play-In-Graph execution visualization
 * - Minimap navigation
 * - Undo/Redo support
 */
class StoryGraphEditor : public EditorPanel
{
public:
    StoryGraphEditor();
    ~StoryGraphEditor() override;

    void update(f64 deltaTime) override;
    void render() override;
    void onResize(i32 width, i32 height) override;

    // Graph management
    void setGraph(StoryGraph* graph);
    [[nodiscard]] StoryGraph* getGraph() const { return m_graph; }

    void newGraph();
    Result<void> openGraph(const std::string& path);
    Result<void> saveGraph(const std::string& path);

    // View control
    void setViewOffset(f32 x, f32 y);
    [[nodiscard]] f32 getViewOffsetX() const { return m_viewX; }
    [[nodiscard]] f32 getViewOffsetY() const { return m_viewY; }

    void setZoom(f32 zoom);
    [[nodiscard]] f32 getZoom() const { return m_zoom; }

    void centerView();
    void fitToContent();
    void focusOnNode(scripting::NodeId id);
    void focusOnSelection();

    // Node operations
    scripting::NodeId addNode(StoryNodeType type, f32 screenX, f32 screenY);
    void deleteSelectedNodes();
    void duplicateSelectedNodes();
    void copySelectedNodes();
    void pasteNodes();
    void cutSelectedNodes();

    // Connection operations
    void startConnection(scripting::NodeId nodeId, const std::string& portId, bool isOutput);
    void updatePendingConnection(f32 screenX, f32 screenY);
    void completeConnection(scripting::NodeId nodeId, const std::string& portId);
    void cancelConnection();
    [[nodiscard]] bool isConnecting() const { return m_isConnecting; }

    // Selection
    void selectNodeAt(f32 screenX, f32 screenY);
    void boxSelect(f32 x1, f32 y1, f32 x2, f32 y2);
    void selectAll();
    void clearSelection();

    // Validation
    void runValidation();
    [[nodiscard]] const ValidationResult& getValidationResult() const { return m_validationResult; }
    void navigateToIssue(const ValidationIssue& issue);

    // Play-In-Graph
    void setPlayMode(bool playing);
    [[nodiscard]] bool isPlayMode() const { return m_isPlayMode; }
    void highlightActiveNode(scripting::NodeId id);
    void markNodeVisited(scripting::NodeId id);
    void clearPlayState();
    void jumpToNode(scripting::NodeId id);  // Force execution to specific node

    // Configuration
    void setConfig(const StoryGraphConfig& config);
    [[nodiscard]] const StoryGraphConfig& getConfig() const { return m_config; }

    // Undo/Redo
    void setUndoSystem(UndoSystem* undoSystem) { m_undoSystem = undoSystem; }
    void undo();
    void redo();

    // Context menu
    void showContextMenu(f32 x, f32 y);
    void hideContextMenu();

    // Node palette (for drag-drop creation)
    void showNodePalette();
    void hideNodePalette();
    [[nodiscard]] bool isNodePaletteVisible() const { return m_showNodePalette; }

    // Callbacks
    void setOnNodeSelected(std::function<void(scripting::NodeId)> callback);
    void setOnNodeDoubleClicked(std::function<void(scripting::NodeId)> callback);
    void setOnGraphModified(std::function<void()> callback);
    void setOnValidationComplete(std::function<void(const ValidationResult&)> callback);
    void setOnJumpToNodeRequested(std::function<void(scripting::NodeId)> callback);

private:
    // Rendering
    void renderBackground();
    void renderGrid();
    void renderConnections();
    void renderPendingConnection();
    void renderNodes();
    void renderSelectionBox();
    void renderMinimap();
    void renderContextMenu();
    void renderNodePalette();
    void renderValidationOverlay();

    // Input handling
    void handleMouseInput();
    void handleKeyboardInput();
    void handleDragDrop();

    // Coordinate conversion
    renderer::Vec2 screenToGraph(f32 screenX, f32 screenY) const;
    renderer::Vec2 graphToScreen(f32 graphX, f32 graphY) const;

    // Hit testing
    scripting::NodeId hitTestNode(f32 x, f32 y) const;
    std::pair<scripting::NodeId, std::string> hitTestPort(f32 x, f32 y) const;
    StoryConnection* hitTestConnection(f32 x, f32 y) const;

    // Bezier curve rendering
    void renderBezierConnection(const StoryConnection& conn);
    std::vector<renderer::Vec2> calculateBezierPoints(
        f32 x1, f32 y1, f32 x2, f32 y2, i32 segments = 32) const;

    // Layout helpers
    void autoLayoutNodes();
    void alignSelectedNodes(bool horizontal);
    void distributeSelectedNodes(bool horizontal);

    StoryGraph* m_graph = nullptr;
    StoryGraphConfig m_config;
    UndoSystem* m_undoSystem = nullptr;

    // View state
    f32 m_viewX = 0.0f;
    f32 m_viewY = 0.0f;
    f32 m_zoom = 1.0f;

    // Interaction state
    enum class DragMode : u8
    {
        None,
        Pan,
        MoveNodes,
        BoxSelect,
        Resize
    };
    DragMode m_dragMode = DragMode::None;
    f32 m_dragStartX = 0.0f;
    f32 m_dragStartY = 0.0f;
    f32 m_dragOffsetX = 0.0f;
    f32 m_dragOffsetY = 0.0f;

    // Connection creation
    bool m_isConnecting = false;
    scripting::NodeId m_connectionSourceNode = 0;
    std::string m_connectionSourcePort;
    bool m_connectionFromOutput = true;
    f32 m_connectionEndX = 0.0f;
    f32 m_connectionEndY = 0.0f;

    // Selection box
    bool m_isBoxSelecting = false;
    f32 m_boxSelectStartX = 0.0f;
    f32 m_boxSelectStartY = 0.0f;
    f32 m_boxSelectEndX = 0.0f;
    f32 m_boxSelectEndY = 0.0f;

    // Validation
    ValidationResult m_validationResult;
    f64 m_validationTimer = 0.0;
    bool m_validationDirty = true;

    // Play mode
    bool m_isPlayMode = false;
    scripting::NodeId m_activeNodeId = 0;

    // Context menu
    bool m_showContextMenu = false;
    f32 m_contextMenuX = 0.0f;
    f32 m_contextMenuY = 0.0f;

    // Node palette
    bool m_showNodePalette = false;
    std::string m_nodeSearchFilter;

    // Clipboard
    std::vector<std::unique_ptr<StoryNode>> m_clipboard;

    // Callbacks
    std::function<void(scripting::NodeId)> m_onNodeSelected;
    std::function<void(scripting::NodeId)> m_onNodeDoubleClicked;
    std::function<void()> m_onGraphModified;
    std::function<void(const ValidationResult&)> m_onValidationComplete;
    std::function<void(scripting::NodeId)> m_onJumpToNodeRequested;
};

/**
 * @brief Node palette categories for organized node creation
 */
namespace StoryNodeCategories
{
    struct Category
    {
        std::string name;
        std::string icon;
        renderer::Color color;
        std::vector<StoryNodeType> nodeTypes;
    };

    std::vector<Category> getCategories();
    const Category* findCategoryForType(StoryNodeType type);
    std::string getNodeIcon(StoryNodeType type);
    std::string getNodeDescription(StoryNodeType type);
}

/**
 * @brief Utility functions for story graph manipulation
 */
namespace StoryGraphUtils
{
    // Auto-layout algorithms
    void layoutHierarchical(StoryGraph& graph);
    void layoutForceDirected(StoryGraph& graph);
    void layoutGrid(StoryGraph& graph);

    // Analysis
    std::vector<scripting::NodeId> findPath(const StoryGraph& graph,
                                            scripting::NodeId from, scripting::NodeId to);
    std::vector<std::vector<scripting::NodeId>> findAllPaths(const StoryGraph& graph,
                                                             scripting::NodeId from, scripting::NodeId to);
    std::vector<scripting::NodeId> getReachableNodes(const StoryGraph& graph, scripting::NodeId from);
    std::vector<scripting::NodeId> getUnreachableNodes(const StoryGraph& graph);

    // Statistics
    struct GraphStats
    {
        i32 totalNodes;
        i32 totalConnections;
        i32 dialogueCount;
        i32 choiceCount;
        i32 branchCount;
        i32 sceneChangeCount;
        i32 maxDepth;
        i32 averageBranchFactor;
    };
    GraphStats calculateStats(const StoryGraph& graph);

    // Export
    std::string exportToMermaid(const StoryGraph& graph);
    std::string exportToDot(const StoryGraph& graph);
}

} // namespace NovelMind::editor
