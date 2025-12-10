#pragma once

/**
 * @file ir.hpp
 * @brief Intermediate Representation (IR) for NM Script
 *
 * The IR serves as the bridge between:
 * - NM Script text (parsed AST)
 * - Visual Graph (node-based editor)
 * - Bytecode (runtime execution)
 *
 * This enables bidirectional conversion:
 * - AST -> IR -> Visual Graph
 * - Visual Graph -> IR -> AST -> NM Script text
 */

#include "NovelMind/core/types.hpp"
#include "NovelMind/core/result.hpp"
#include "NovelMind/scripting/ast.hpp"
#include "NovelMind/scripting/parser.hpp"
#include "NovelMind/scripting/lexer.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <optional>

namespace NovelMind::scripting
{

// Forward declarations
class IRNode;
class IRGraph;

/**
 * @brief Unique identifier for IR nodes
 */
using NodeId = u64;

/**
 * @brief Port identifier for node connections
 */
struct PortId
{
    NodeId nodeId;
    std::string portName;
    bool isOutput;

    bool operator==(const PortId& other) const
    {
        return nodeId == other.nodeId && portName == other.portName && isOutput == other.isOutput;
    }
};

/**
 * @brief Connection between two nodes
 */
struct IRConnection
{
    PortId source;      // Output port
    PortId target;      // Input port
    std::string label;  // Optional label for debugging
};

/**
 * @brief IR Node types
 */
enum class IRNodeType : u8
{
    // Structure
    SceneStart,         // Entry point for a scene
    SceneEnd,           // Exit point
    Comment,            // Documentation node

    // Flow control
    Sequence,           // Sequential execution
    Branch,             // Conditional branch (if/else)
    Switch,             // Multi-way branch
    Loop,               // Loop construct
    Goto,               // Jump to label/scene
    Label,              // Jump target

    // VN-specific
    ShowCharacter,      // Display character
    HideCharacter,      // Hide character
    ShowBackground,     // Display background
    Dialogue,           // Say text
    Choice,             // Player choice
    ChoiceOption,       // Single choice option

    // Audio
    PlayMusic,          // Play music track
    StopMusic,          // Stop music
    PlaySound,          // Play sound effect

    // Effects
    Transition,         // Scene transition effect
    Wait,               // Delay execution
    SetVariable,        // Set variable value
    GetVariable,        // Read variable value

    // Advanced
    Expression,         // Expression evaluation
    FunctionCall,       // Call a function
    Custom              // User-defined node
};

/**
 * @brief Property value types for IR nodes
 */
using IRPropertyValue = std::variant<
    std::nullptr_t,
    bool,
    i64,
    f64,
    std::string,
    std::vector<std::string>
>;

/**
 * @brief Port definition for a node type
 */
struct PortDefinition
{
    std::string name;
    std::string displayName;
    bool isExecution = false;   // True for flow ports, false for data ports
    bool required = false;
    std::string defaultValue;
};

/**
 * @brief IR Node - base unit of the IR graph
 */
class IRNode
{
public:
    IRNode(NodeId id, IRNodeType type);
    virtual ~IRNode() = default;

    // Identity
    [[nodiscard]] NodeId getId() const { return m_id; }
    [[nodiscard]] IRNodeType getType() const { return m_type; }
    [[nodiscard]] const char* getTypeName() const;

    // Properties
    void setProperty(const std::string& name, const IRPropertyValue& value);
    [[nodiscard]] std::optional<IRPropertyValue> getProperty(const std::string& name) const;
    [[nodiscard]] const std::unordered_map<std::string, IRPropertyValue>& getProperties() const { return m_properties; }

    // Convenience property accessors
    [[nodiscard]] std::string getStringProperty(const std::string& name, const std::string& defaultValue = "") const;
    [[nodiscard]] i64 getIntProperty(const std::string& name, i64 defaultValue = 0) const;
    [[nodiscard]] f64 getFloatProperty(const std::string& name, f64 defaultValue = 0.0) const;
    [[nodiscard]] bool getBoolProperty(const std::string& name, bool defaultValue = false) const;

    // Source location (for error reporting and debugging)
    void setSourceLocation(const SourceLocation& loc);
    [[nodiscard]] const SourceLocation& getSourceLocation() const { return m_location; }

    // Visual position (for graph editor)
    void setPosition(f32 x, f32 y);
    [[nodiscard]] f32 getX() const { return m_x; }
    [[nodiscard]] f32 getY() const { return m_y; }

    // Port definitions (based on type)
    [[nodiscard]] std::vector<PortDefinition> getInputPorts() const;
    [[nodiscard]] std::vector<PortDefinition> getOutputPorts() const;

    // Serialization
    [[nodiscard]] std::string toJson() const;
    static std::unique_ptr<IRNode> fromJson(const std::string& json);

protected:
    NodeId m_id;
    IRNodeType m_type;
    std::unordered_map<std::string, IRPropertyValue> m_properties;
    SourceLocation m_location;
    f32 m_x = 0.0f;
    f32 m_y = 0.0f;
};

/**
 * @brief IR Graph - complete representation of a script
 */
class IRGraph
{
public:
    IRGraph();
    ~IRGraph();

    // Graph identity
    void setName(const std::string& name);
    [[nodiscard]] const std::string& getName() const { return m_name; }

    // Node management
    NodeId createNode(IRNodeType type);
    void removeNode(NodeId id);
    [[nodiscard]] IRNode* getNode(NodeId id);
    [[nodiscard]] const IRNode* getNode(NodeId id) const;
    [[nodiscard]] std::vector<IRNode*> getNodes();
    [[nodiscard]] std::vector<const IRNode*> getNodes() const;
    [[nodiscard]] std::vector<IRNode*> getNodesByType(IRNodeType type);

    // Connection management
    Result<void> connect(const PortId& source, const PortId& target);
    void disconnect(const PortId& source, const PortId& target);
    void disconnectAll(NodeId nodeId);
    [[nodiscard]] std::vector<IRConnection> getConnections() const;
    [[nodiscard]] std::vector<IRConnection> getConnectionsFrom(NodeId nodeId) const;
    [[nodiscard]] std::vector<IRConnection> getConnectionsTo(NodeId nodeId) const;
    [[nodiscard]] bool isConnected(const PortId& source, const PortId& target) const;

    // Traversal
    [[nodiscard]] std::vector<NodeId> getTopologicalOrder() const;
    [[nodiscard]] std::vector<NodeId> getExecutionOrder() const;

    // Validation
    [[nodiscard]] std::vector<std::string> validate() const;
    [[nodiscard]] bool isValid() const;

    // Scene management (for multi-scene scripts)
    void addScene(const std::string& sceneName, NodeId startNode);
    [[nodiscard]] NodeId getSceneStartNode(const std::string& sceneName) const;
    [[nodiscard]] std::vector<std::string> getSceneNames() const;

    // Character declarations
    void addCharacter(const std::string& id, const std::string& name, const std::string& color);
    [[nodiscard]] bool hasCharacter(const std::string& id) const;

    // Serialization
    [[nodiscard]] std::string toJson() const;
    static std::unique_ptr<IRGraph> fromJson(const std::string& json);

private:
    NodeId m_nextId = 1;
    std::string m_name;
    std::unordered_map<NodeId, std::unique_ptr<IRNode>> m_nodes;
    std::vector<IRConnection> m_connections;
    std::unordered_map<std::string, NodeId> m_sceneStartNodes;
    std::unordered_map<std::string, std::pair<std::string, std::string>> m_characters; // id -> (name, color)
};

/**
 * @brief Converts AST to IR
 */
class ASTToIRConverter
{
public:
    ASTToIRConverter();
    ~ASTToIRConverter();

    /**
     * @brief Convert parsed AST to IR graph
     */
    Result<std::unique_ptr<IRGraph>> convert(const Program& program);

private:
    void convertCharacterDecl(const CharacterDecl& decl);
    NodeId convertScene(const SceneDecl& scene);
    NodeId convertStatement(const Statement& stmt, NodeId prevNode);
    NodeId convertExpression(const Expression& expr);

    NodeId createNodeAndConnect(IRNodeType type, NodeId prevNode);

    std::unique_ptr<IRGraph> m_graph;
    f32 m_currentY = 0.0f;
    f32 m_nodeSpacing = 100.0f;
};

/**
 * @brief Converts IR to AST (for code generation)
 */
class IRToASTConverter
{
public:
    IRToASTConverter();
    ~IRToASTConverter();

    /**
     * @brief Convert IR graph back to AST
     */
    Result<Program> convert(const IRGraph& graph);

private:
    std::unique_ptr<Statement> convertNode(const IRNode* node, const IRGraph& graph);
    std::unique_ptr<Expression> convertToExpression(const IRNode* node, const IRGraph& graph);

    std::unordered_set<NodeId> m_visited;
};

/**
 * @brief Generates NM Script text from AST
 */
class ASTToTextGenerator
{
public:
    ASTToTextGenerator();
    ~ASTToTextGenerator();

    /**
     * @brief Generate NM Script text from AST
     */
    std::string generate(const Program& program);

private:
    void generateCharacter(const CharacterDecl& decl);
    void generateScene(const SceneDecl& scene);
    void generateStatement(const Statement& stmt, int indent);
    void generateExpression(const Expression& expr);

    std::string m_output;
    int m_indentLevel = 0;

    void indent();
    void newline();
    void write(const std::string& text);
};

/**
 * @brief Visual Graph node for editor representation
 */
struct VisualGraphNode
{
    NodeId id;
    std::string type;
    std::string displayName;
    f32 x;
    f32 y;
    f32 width = 200.0f;
    f32 height = 100.0f;
    std::vector<std::pair<std::string, std::string>> inputPorts;  // name, displayName
    std::vector<std::pair<std::string, std::string>> outputPorts; // name, displayName
    std::unordered_map<std::string, std::string> properties;
    bool selected = false;
    bool collapsed = false;
};

/**
 * @brief Visual Graph edge for editor representation
 */
struct VisualGraphEdge
{
    NodeId sourceNode;
    std::string sourcePort;
    NodeId targetNode;
    std::string targetPort;
    bool selected = false;
};

/**
 * @brief Visual Graph - editor-friendly representation
 */
class VisualGraph
{
public:
    VisualGraph();
    ~VisualGraph();

    // Conversion from/to IR
    void fromIR(const IRGraph& ir);
    std::unique_ptr<IRGraph> toIR() const;

    // Node access
    [[nodiscard]] const std::vector<VisualGraphNode>& getNodes() const { return m_nodes; }
    [[nodiscard]] const std::vector<VisualGraphEdge>& getEdges() const { return m_edges; }

    [[nodiscard]] VisualGraphNode* findNode(NodeId id);
    [[nodiscard]] const VisualGraphNode* findNode(NodeId id) const;

    // Node manipulation
    NodeId addNode(const std::string& type, f32 x, f32 y);
    void removeNode(NodeId id);
    void setNodePosition(NodeId id, f32 x, f32 y);
    void setNodeProperty(NodeId id, const std::string& name, const std::string& value);

    // Edge manipulation
    void addEdge(NodeId sourceNode, const std::string& sourcePort,
                 NodeId targetNode, const std::string& targetPort);
    void removeEdge(NodeId sourceNode, const std::string& sourcePort,
                    NodeId targetNode, const std::string& targetPort);

    // Selection
    void selectNode(NodeId id, bool addToSelection = false);
    void deselectNode(NodeId id);
    void selectEdge(NodeId sourceNode, const std::string& sourcePort,
                    NodeId targetNode, const std::string& targetPort);
    void clearSelection();

    // Layout
    void autoLayout();

    // Serialization
    [[nodiscard]] std::string toJson() const;
    static std::unique_ptr<VisualGraph> fromJson(const std::string& json);

private:
    NodeId m_nextId = 1;
    std::vector<VisualGraphNode> m_nodes;
    std::vector<VisualGraphEdge> m_edges;
};

/**
 * @brief Round-trip converter between all representations
 *
 * Provides bidirectional conversion:
 * - NM Script text <-> AST <-> IR <-> Visual Graph
 */
class RoundTripConverter
{
public:
    RoundTripConverter();
    ~RoundTripConverter();

    // Text -> IR
    Result<std::unique_ptr<IRGraph>> textToIR(const std::string& nmScript);

    // IR -> Text
    Result<std::string> irToText(const IRGraph& ir);

    // IR <-> Visual Graph
    Result<std::unique_ptr<VisualGraph>> irToVisualGraph(const IRGraph& ir);
    Result<std::unique_ptr<IRGraph>> visualGraphToIR(const VisualGraph& graph);

    // Direct conversions
    Result<std::unique_ptr<VisualGraph>> textToVisualGraph(const std::string& nmScript);
    Result<std::string> visualGraphToText(const VisualGraph& graph);

    // Validation
    [[nodiscard]] std::vector<std::string> validateConversion(const std::string& original,
                                                               const std::string& roundTripped);

private:
    // Components
    std::unique_ptr<Lexer> m_lexer;
    std::unique_ptr<Parser> m_parser;
    std::unique_ptr<ASTToIRConverter> m_astToIR;
    std::unique_ptr<IRToASTConverter> m_irToAST;
    std::unique_ptr<ASTToTextGenerator> m_textGen;
};

} // namespace NovelMind::scripting
