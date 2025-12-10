#pragma once

/**
 * @file play_visualization.hpp
 * @brief Play-In-Editor State Visualization
 *
 * Enhanced debugging visualization for Play-In-Editor mode:
 * - Active StoryGraph node highlighting
 * - Current dialogue/replica display
 * - Active animation visualization
 * - AudioManager state display
 * - ScriptRuntime stack visualization
 */

#include "NovelMind/core/types.hpp"
#include "NovelMind/scripting/ir.hpp"
#include "NovelMind/renderer/renderer.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <deque>

namespace NovelMind::editor
{

// Forward declarations
class EditorRuntimeHost;

/**
 * @brief State of an animation for visualization
 */
struct AnimationVisualState
{
    std::string name;
    std::string targetObject;
    std::string property;
    f32 progress;       // 0.0 - 1.0
    f32 duration;
    f32 elapsed;
    std::string easingType;
    bool isPlaying;
    bool isLooping;
};

/**
 * @brief State of an audio track for visualization
 */
struct AudioVisualState
{
    enum class Type : u8
    {
        BGM,
        SFX,
        Voice,
        Ambient
    };

    Type type;
    std::string name;
    std::string filePath;
    f32 volume;
    f32 progress;       // 0.0 - 1.0
    f32 duration;
    bool isPlaying;
    bool isLooping;
    bool isFading;
};

/**
 * @brief State of a variable for visualization
 */
struct VariableVisualState
{
    std::string name;
    std::string value;
    std::string type;
    bool wasModifiedThisFrame;
    u64 lastModifiedFrame;
};

/**
 * @brief State of a flag for visualization
 */
struct FlagVisualState
{
    std::string name;
    bool value;
    bool wasModifiedThisFrame;
    u64 lastModifiedFrame;
};

/**
 * @brief Stack frame for visualization
 */
struct StackFrameVisual
{
    std::string sceneName;
    std::string functionName;
    i32 lineNumber;
    i32 instructionPointer;
    std::vector<VariableVisualState> localVariables;
};

/**
 * @brief Dialogue state for visualization
 */
struct DialogueVisualState
{
    std::string speakerName;
    std::string speakerId;
    std::string currentText;
    std::string fullText;
    i32 displayedCharacters;
    bool isTypewriting;
    bool isWaitingForInput;
    std::string voiceFile;
    bool hasVoice;
};

/**
 * @brief Choice state for visualization
 */
struct ChoiceVisualState
{
    struct Option
    {
        std::string text;
        bool isEnabled;
        bool isHovered;
        std::string condition;  // Condition that enables this option
    };

    bool isActive;
    std::vector<Option> options;
    i32 selectedIndex;
};

/**
 * @brief Node highlight state for StoryGraph
 */
struct NodeHighlightState
{
    scripting::NodeId nodeId;
    bool isActive;          // Currently executing
    bool wasVisited;        // Was executed in this session
    bool isBreakpoint;      // Has breakpoint set
    bool isPaused;          // Execution paused here
    renderer::Color highlightColor;
};

/**
 * @brief Complete runtime visualization state
 */
struct RuntimeVisualizationState
{
    // Current execution state
    scripting::NodeId activeNodeId = 0;
    std::string activeSceneName;
    i32 currentLine = -1;
    i32 currentInstructionPointer = 0;

    // Dialogue state
    DialogueVisualState dialogue;
    ChoiceVisualState choices;

    // Stack
    std::vector<StackFrameVisual> callStack;

    // Variables and flags
    std::vector<VariableVisualState> globalVariables;
    std::vector<FlagVisualState> flags;

    // Animations
    std::vector<AnimationVisualState> activeAnimations;

    // Audio
    std::vector<AudioVisualState> activeAudio;

    // Node history
    std::deque<scripting::NodeId> nodeHistory;
    size_t maxHistorySize = 50;

    // Performance metrics
    f64 scriptExecutionTimeMs = 0.0;
    i32 instructionsThisFrame = 0;
    i32 totalInstructionsExecuted = 0;
};

/**
 * @brief Configuration for visualization display
 */
struct VisualizationConfig
{
    // StoryGraph highlighting
    bool showActiveNode = true;
    bool showVisitedNodes = true;
    bool showBreakpoints = true;
    bool showNodeHistory = true;

    renderer::Color activeNodeColor = {0.2f, 0.8f, 0.2f, 1.0f};      // Green
    renderer::Color visitedNodeColor = {0.5f, 0.5f, 0.8f, 0.5f};     // Blue
    renderer::Color breakpointColor = {0.9f, 0.2f, 0.2f, 1.0f};      // Red
    renderer::Color pausedColor = {0.9f, 0.8f, 0.2f, 1.0f};          // Yellow

    // Dialogue display
    bool showDialogueOverlay = true;
    bool showTypewriterProgress = true;
    bool showVoiceWaveform = true;

    // Animation display
    bool showAnimationPanel = true;
    bool showAnimationProgress = true;
    bool highlightAnimatedObjects = true;

    // Audio display
    bool showAudioPanel = true;
    bool showWaveforms = true;
    bool showVolumeMeters = true;

    // Variables display
    bool showVariablesPanel = true;
    bool highlightModifiedVariables = true;
    f32 modifiedHighlightDuration = 1.0f;  // seconds

    // Stack display
    bool showCallStack = true;
    bool showLocalVariables = true;

    // Performance overlay
    bool showPerformanceMetrics = true;
};

/**
 * @brief Listener for visualization state changes
 */
class IVisualizationListener
{
public:
    virtual ~IVisualizationListener() = default;

    virtual void onActiveNodeChanged(scripting::NodeId newNode) = 0;
    virtual void onDialogueChanged(const DialogueVisualState& dialogue) = 0;
    virtual void onChoicesShown(const ChoiceVisualState& choices) = 0;
    virtual void onVariableModified(const std::string& name, const std::string& value) = 0;
    virtual void onAnimationStarted(const AnimationVisualState& animation) = 0;
    virtual void onAnimationEnded(const std::string& animationName) = 0;
    virtual void onAudioStarted(const AudioVisualState& audio) = 0;
    virtual void onAudioEnded(const std::string& audioName) = 0;
};

/**
 * @brief Play-In-Editor Visualization Manager
 *
 * Provides comprehensive visualization of the runtime state during Play-In-Editor:
 * - Highlights the active node in StoryGraph
 * - Shows current dialogue/replica state
 * - Displays active animations with progress
 * - Shows AudioManager state (BGM, SFX, Voice)
 * - Visualizes ScriptRuntime call stack
 */
class PlayVisualizationManager
{
public:
    PlayVisualizationManager();
    ~PlayVisualizationManager() = default;

    /**
     * @brief Initialize with runtime host
     */
    void initialize(EditorRuntimeHost* runtimeHost);

    /**
     * @brief Update visualization state from runtime
     */
    void update(f64 deltaTime);

    /**
     * @brief Get current visualization state
     */
    [[nodiscard]] const RuntimeVisualizationState& getState() const { return m_state; }

    /**
     * @brief Get node highlights for StoryGraph rendering
     */
    [[nodiscard]] std::vector<NodeHighlightState> getNodeHighlights() const;

    /**
     * @brief Set visualization configuration
     */
    void setConfig(const VisualizationConfig& config);

    /**
     * @brief Get current configuration
     */
    [[nodiscard]] const VisualizationConfig& getConfig() const { return m_config; }

    /**
     * @brief Add visualization listener
     */
    void addListener(IVisualizationListener* listener);

    /**
     * @brief Remove visualization listener
     */
    void removeListener(IVisualizationListener* listener);

    /**
     * @brief Reset visualization state
     */
    void reset();

    /**
     * @brief Mark a node as visited
     */
    void markNodeVisited(scripting::NodeId nodeId);

    /**
     * @brief Clear visited node history
     */
    void clearVisitedNodes();

    /**
     * @brief Get execution trace for debugging
     */
    [[nodiscard]] std::vector<std::pair<scripting::NodeId, u64>> getExecutionTrace() const;

    // Rendering helpers
    /**
     * @brief Render dialogue overlay
     */
    void renderDialogueOverlay(renderer::IRenderer* renderer);

    /**
     * @brief Render animation panel
     */
    void renderAnimationPanel(renderer::IRenderer* renderer);

    /**
     * @brief Render audio panel
     */
    void renderAudioPanel(renderer::IRenderer* renderer);

    /**
     * @brief Render variables panel
     */
    void renderVariablesPanel(renderer::IRenderer* renderer);

    /**
     * @brief Render call stack panel
     */
    void renderCallStackPanel(renderer::IRenderer* renderer);

    /**
     * @brief Render performance overlay
     */
    void renderPerformanceOverlay(renderer::IRenderer* renderer);

private:
    void updateDialogueState();
    void updateChoiceState();
    void updateAnimationState();
    void updateAudioState();
    void updateVariableState();
    void updateStackState();
    void recordNodeHistory(scripting::NodeId nodeId);

    void notifyActiveNodeChanged(scripting::NodeId nodeId);
    void notifyDialogueChanged();
    void notifyChoicesShown();
    void notifyVariableModified(const std::string& name, const std::string& value);
    void notifyAnimationStarted(const AnimationVisualState& animation);
    void notifyAnimationEnded(const std::string& name);
    void notifyAudioStarted(const AudioVisualState& audio);
    void notifyAudioEnded(const std::string& name);

    EditorRuntimeHost* m_runtimeHost = nullptr;
    RuntimeVisualizationState m_state;
    VisualizationConfig m_config;

    std::vector<IVisualizationListener*> m_listeners;
    std::unordered_set<scripting::NodeId> m_visitedNodes;

    // For tracking changes
    scripting::NodeId m_lastActiveNode = 0;
    std::string m_lastDialogueText;
    i32 m_lastChoiceCount = 0;

    // Performance tracking
    u64 m_frameCounter = 0;
    f64 m_lastUpdateTime = 0.0;
};

/**
 * @brief StoryGraph Visualization Overlay
 *
 * Renders visual indicators on the StoryGraph during Play-In-Editor:
 * - Active node highlight (pulsing glow)
 * - Visited path (dimmed trail)
 * - Breakpoint markers
 * - Execution flow arrows
 */
class StoryGraphVisualizationOverlay
{
public:
    StoryGraphVisualizationOverlay();
    ~StoryGraphVisualizationOverlay() = default;

    /**
     * @brief Set the visualization manager
     */
    void setVisualizationManager(PlayVisualizationManager* manager);

    /**
     * @brief Update animation state
     */
    void update(f64 deltaTime);

    /**
     * @brief Render overlay on StoryGraph
     */
    void render(renderer::IRenderer* renderer, f32 viewX, f32 viewY, f32 zoom);

    /**
     * @brief Set node position for rendering
     */
    void setNodePosition(scripting::NodeId nodeId, f32 x, f32 y, f32 width, f32 height);

    /**
     * @brief Clear cached node positions
     */
    void clearNodePositions();

private:
    void renderActiveNodeHighlight(renderer::IRenderer* renderer,
                                   scripting::NodeId nodeId,
                                   f32 viewX, f32 viewY, f32 zoom);
    void renderVisitedPath(renderer::IRenderer* renderer,
                           f32 viewX, f32 viewY, f32 zoom);
    void renderBreakpointMarkers(renderer::IRenderer* renderer,
                                 f32 viewX, f32 viewY, f32 zoom);
    void renderExecutionFlowArrows(renderer::IRenderer* renderer,
                                   f32 viewX, f32 viewY, f32 zoom);

    PlayVisualizationManager* m_manager = nullptr;

    struct NodeBounds
    {
        f32 x, y, width, height;
    };
    std::unordered_map<scripting::NodeId, NodeBounds> m_nodePositions;

    // Animation state
    f32 m_pulsePhase = 0.0f;
    f32 m_arrowPhase = 0.0f;
};

} // namespace NovelMind::editor
