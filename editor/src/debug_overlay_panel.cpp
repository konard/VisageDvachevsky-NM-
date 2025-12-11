/**
 * @file debug_overlay_panel.cpp
 * @brief Debug Overlay Panel implementation
 */

#include "NovelMind/editor/debug_overlay_panel.hpp"
#include "NovelMind/editor/imgui_integration.hpp"
#include "NovelMind/editor/style_guide.hpp"
#include <algorithm>
#include <cstdio>

namespace NovelMind::editor {

DebugOverlayPanel::DebugOverlayPanel()
    : GUIPanelBase("Debug Overlay")
{
    m_frameTimeHistory.resize(FrameHistorySize, 0.0f);

    // Register as play mode listener
    PlayModeController::instance().addListener(this);
}

DebugOverlayPanel::~DebugOverlayPanel()
{
    PlayModeController::instance().removeListener(this);
}

void DebugOverlayPanel::onInitialize()
{
}

void DebugOverlayPanel::onShutdown()
{
}

void DebugOverlayPanel::onUpdate(f64 deltaTime)
{
    // Update frame time history
    m_frameTimeHistory[m_frameHistoryIndex] = static_cast<f32>(deltaTime * 1000.0);  // Convert to ms
    m_frameHistoryIndex = (m_frameHistoryIndex + 1) % FrameHistorySize;
}

void DebugOverlayPanel::onRender()
{
    // Only show when in play mode
    if (!PlayModeController::instance().isInPlayMode() && !m_hasError) {
        // Could show a "not in play mode" message
        return;
    }

    // Apply semi-transparent style if configured
    if (m_config.opacity < 1.0f) {
        // Would push style for transparency
    }

    // Render sections based on config
    if (m_config.showPerformance) {
        renderCollapsibleSection("Performance", &m_performanceOpen, [this]() {
            renderPerformance();
        });
    }

    if (m_config.showNodeHighlight) {
        renderCollapsibleSection("Current Node", &m_nodeHighlightOpen, [this]() {
            renderNodeHighlight();
        });
    }

    if (m_config.showVMState) {
        renderCollapsibleSection("VM State", &m_vmStateOpen, [this]() {
            renderVMState();
        });
    }

    if (m_config.showCallStack) {
        renderCollapsibleSection("Call Stack", &m_callStackOpen, [this]() {
            renderCallStack();
        });
    }

    if (m_config.showVariables) {
        renderCollapsibleSection("Variables", &m_variablesOpen, [this]() {
            renderVariables();
        });
    }

    if (m_config.showAnimations) {
        renderCollapsibleSection("Animations", &m_animationsOpen, [this]() {
            renderAnimations();
        });
    }

    if (m_config.showAudioChannels) {
        renderCollapsibleSection("Audio", &m_audioOpen, [this]() {
            renderAudioChannels();
        });
    }

    if (m_config.showTimelineState) {
        renderCollapsibleSection("Timeline", &m_timelineOpen, [this]() {
            renderTimelineState();
        });
    }

    // Show error banner if there's an error
    if (m_hasError) {
        // Red error banner with message
        // ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.3f, 0.3f, 1));
        // ImGui::Text("Error: %s", m_errorMessage.c_str());
        // ImGui::PopStyleColor();
    }
}

void DebugOverlayPanel::renderToolbar()
{
    widgets::BeginToolbar("DebugOverlayToolbar", Heights::Toolbar);

    // Play/Pause/Stop buttons
    auto& controller = PlayModeController::instance();

    if (controller.isPlaying()) {
        if (widgets::ToolbarButton(Icons::Pause, "Pause")) {
            controller.pause();
        }
    } else if (controller.isPaused()) {
        if (widgets::ToolbarButton(Icons::Play, "Resume")) {
            controller.resume();
        }
    }

    if (controller.isInPlayMode()) {
        if (widgets::ToolbarButton(Icons::Stop, "Stop")) {
            controller.stop();
        }

        widgets::ToolbarSeparator();

        // Step controls (only when paused)
        if (controller.isPaused()) {
            if (widgets::ToolbarButton(Icons::StepForward, "Step")) {
                controller.stepNext();
            }
        }
    }

    widgets::ToolbarSeparator();

    // Toggle sections
    widgets::ToolbarToggle(Icons::Visible, &m_config.showPerformance, "Performance");
    widgets::ToolbarToggle(Icons::Node, &m_config.showNodeHighlight, "Node");
    widgets::ToolbarToggle(Icons::Script, &m_config.showVMState, "VM");
    widgets::ToolbarToggle(Icons::Tree, &m_config.showCallStack, "Call Stack");

    widgets::EndToolbar();
}

void DebugOverlayPanel::renderCollapsibleSection(const char* title, bool* isOpen,
                                                  std::function<void()> content)
{
    if (widgets::CollapsingHeader(title, isOpen, *isOpen)) {
        content();
    }
}

void DebugOverlayPanel::renderPerformance()
{
    if (m_config.compactMode) {
        // Compact: single line with key metrics
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "FPS: %.1f | Frame: %.2fms | Draw: %u",
                 m_metrics.fps, m_metrics.frameTime * 1000.0, m_metrics.drawCalls);
        // ImGui::Text("%s", buffer);
        (void)buffer;
    } else {
        // Full view with graph

        // FPS counter
        // ImGui::Text("FPS: %.1f", m_metrics.fps);

        // Frame time graph
        // Would render a line graph of m_frameTimeHistory

        // Detailed stats
        // ImGui::Text("Frame Time: %.2f ms", m_metrics.frameTime * 1000.0);
        // ImGui::Text("CPU Time: %.2f ms", m_metrics.cpuTime * 1000.0);
        // ImGui::Text("Draw Calls: %u", m_metrics.drawCalls);
        // ImGui::Text("Triangles: %u", m_metrics.triangles);
        // ImGui::Text("Memory: %.1f MB", m_metrics.memoryUsage / (1024.0 * 1024.0));
    }
}

void DebugOverlayPanel::renderNodeHighlight()
{
    // Current node info
    if (!m_currentNodeId.empty()) {
        // ImGui::Text("Current: %s", m_currentNodeName.c_str());
        // ImGui::SameLine();

        // Navigate button
        if (widgets::ToolbarButton(Icons::ArrowRight, "Go to node")) {
            if (m_onNavigateToNode) {
                m_onNavigateToNode(m_currentNodeId);
            }
        }
    } else {
        // ImGui::TextDisabled("No active node");
    }

    // Next possible nodes (branching)
    if (!m_nextNodes.empty()) {
        widgets::SectionHeader("Next Nodes:");
        for (const auto& [id, name] : m_nextNodes) {
            // ImGui::BulletText("%s", name.c_str());
            (void)name;
        }
    }
}

void DebugOverlayPanel::renderVMState()
{
    // Registers
    // ImGui::Text("PC: %u | SP: %u | FP: %u", m_pc, m_sp, m_fp);

    // Current instruction area
    if (!m_vmInstructions.empty()) {
        widgets::SectionHeader("Instructions:");

        // Show a few instructions around current
        for (const auto& instr : m_vmInstructions) {
            bool isCurrent = instr.isCurrent;
            bool hasBreakpoint = instr.hasBreakpoint;

            // Format: [BP] > 0042: OPCODE operand
            char prefix[8] = "   ";
            if (hasBreakpoint) prefix[0] = 'B';
            if (isCurrent) prefix[1] = '>';

            // ImGui::Text("%s %04u: %s %s", prefix, instr.index,
            //             instr.opcode.c_str(), instr.operand.c_str());

            if (isCurrent) {
                // Highlight current line
            }

            (void)prefix;
        }
    }
}

void DebugOverlayPanel::renderCallStack()
{
    if (m_callStack.empty()) {
        // ImGui::TextDisabled("Empty call stack");
        return;
    }

    for (i32 i = static_cast<i32>(m_callStack.size()) - 1; i >= 0; --i) {
        const auto& frame = m_callStack[static_cast<size_t>(i)];
        bool isSelected = (i == m_selectedStackFrame);

        // Frame entry: function name @ file:line
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s%s @ %s:%u",
                 frame.isCurrentFrame ? "> " : "  ",
                 frame.functionName.c_str(),
                 frame.sourceFile.c_str(),
                 frame.lineNumber);

        // Make selectable
        // if (ImGui::Selectable(buffer, isSelected)) {
        //     m_selectedStackFrame = i;
        //     if (m_onSelectFrame) {
        //         m_onSelectFrame(i);
        //     }
        // }

        // Double-click to navigate
        // if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        //     if (m_onNavigateToSource) {
        //         m_onNavigateToSource(frame.sourceFile, frame.lineNumber);
        //     }
        // }

        (void)buffer;
        (void)isSelected;
    }
}

void DebugOverlayPanel::renderVariables()
{
    // Watch input
    // ImGui::InputText("##WatchInput", m_watchInputBuffer, sizeof(m_watchInputBuffer));
    // ImGui::SameLine();
    // if (ImGui::Button("Add Watch")) {
    //     addWatch(m_watchInputBuffer);
    //     m_watchInputBuffer[0] = '\0';
    // }

    widgets::SectionHeader("Watch:");

    // Pinned variables first
    for (const auto& var : m_variables) {
        if (!var.isPinned) continue;
        renderVariableEntry(var);
    }

    // Then unpinned
    for (const auto& var : m_variables) {
        if (var.isPinned) continue;
        renderVariableEntry(var);
    }
}

void DebugOverlayPanel::renderAnimations()
{
    if (m_animations.empty()) {
        // ImGui::TextDisabled("No active animations");
        return;
    }

    for (const auto& anim : m_animations) {
        // Animation name and target
        // ImGui::Text("%s -> %s.%s", anim.name.c_str(),
        //             anim.targetObject.c_str(), anim.property.c_str());

        // Progress bar
        renderProgressBar(anim.progress, 100.0f, DarkPalette::Primary);

        // Duration and state
        // ImGui::SameLine();
        // ImGui::Text("%.1fs %s%s", anim.duration,
        //             anim.isPlaying ? "Playing" : "Paused",
        //             anim.isLooping ? " [Loop]" : "");
    }
}

void DebugOverlayPanel::renderAudioChannels()
{
    if (m_audioChannels.empty()) {
        // ImGui::TextDisabled("No active audio");
        return;
    }

    for (const auto& channel : m_audioChannels) {
        // Channel type icon
        const char* icon = channel.isVoice ? Icons::Voice : Icons::Audio;

        // ImGui::Text("%s [%d] %s", icon, channel.channelId, channel.soundName.c_str());

        // Volume bar
        renderVolumeBar(channel.volume, 80.0f);

        // Position in track
        if (channel.duration > 0) {
            f32 progress = channel.position / channel.duration;
            // ImGui::SameLine();
            // ImGui::Text("%.1f/%.1fs", channel.position, channel.duration);
            renderProgressBar(progress, 60.0f, DarkPalette::Info);
        }

        (void)icon;
    }
}

void DebugOverlayPanel::renderTimelineState()
{
    // Global playback time
    // ImGui::Text("Time: %.2fs", m_playbackTime);

    if (m_timelineTracks.empty()) {
        // ImGui::TextDisabled("No active tracks");
        return;
    }

    for (const auto& track : m_timelineTracks) {
        // Track name and type
        // ImGui::Text("%s (%s)", track.trackName.c_str(), track.trackType.c_str());

        // Progress in track
        if (track.duration > 0) {
            f32 progress = track.currentTime / track.duration;
            renderProgressBar(progress, 100.0f, DarkPalette::TimelineKeyframe);
        }

        // Keyframe info
        if (track.currentKeyframe >= 0) {
            // ImGui::SameLine();
            // ImGui::Text("KF: %d -> %d", track.currentKeyframe, track.nextKeyframe);
        }
    }
}

void DebugOverlayPanel::renderProgressBar(f32 progress, f32 width, const renderer::Color& color)
{
    // Custom progress bar with color
    // ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a/255.0f));
    // ImGui::ProgressBar(progress, ImVec2(width, 0));
    // ImGui::PopStyleColor();

    (void)progress;
    (void)width;
    (void)color;
}

void DebugOverlayPanel::renderVolumeBar(f32 volume, f32 width)
{
    // Volume meter style bar
    renderer::Color color = (volume > 0.8f) ? DarkPalette::Warning :
                            (volume > 0.5f) ? DarkPalette::Success :
                            DarkPalette::Primary;
    renderProgressBar(volume, width, color);
}

renderer::Color DebugOverlayPanel::getValueTypeColor(const std::string& type) const
{
    if (type == "string") return DarkPalette::Success;
    if (type == "number" || type == "int" || type == "float") return DarkPalette::Info;
    if (type == "bool") return DarkPalette::Secondary;
    if (type == "null" || type == "undefined") return DarkPalette::TextTertiary;
    return DarkPalette::Text;
}

void DebugOverlayPanel::setConfig(const DebugOverlayConfig& config)
{
    m_config = config;
}

void DebugOverlayPanel::setVMInstructions(const std::vector<VMInstructionInfo>& instructions)
{
    m_vmInstructions = instructions;
}

void DebugOverlayPanel::setCurrentInstruction(u32 index)
{
    m_currentInstruction = index;
    for (auto& instr : m_vmInstructions) {
        instr.isCurrent = (instr.index == index);
    }
}

void DebugOverlayPanel::setVMRegisters(u32 pc, u32 sp, u32 fp)
{
    m_pc = pc;
    m_sp = sp;
    m_fp = fp;
}

void DebugOverlayPanel::setCallStack(const std::vector<CallStackFrame>& frames)
{
    m_callStack = frames;
}

void DebugOverlayPanel::setCurrentFrame(i32 frameIndex)
{
    m_currentFrame = frameIndex;
    for (size_t i = 0; i < m_callStack.size(); ++i) {
        m_callStack[i].isCurrentFrame = (static_cast<i32>(i) == frameIndex);
    }
}

void DebugOverlayPanel::setVariables(const std::vector<WatchVariable>& variables)
{
    m_variables = variables;
}

void DebugOverlayPanel::addWatch(const std::string& variableName)
{
    if (std::find(m_userWatches.begin(), m_userWatches.end(), variableName) == m_userWatches.end()) {
        m_userWatches.push_back(variableName);
    }
}

void DebugOverlayPanel::removeWatch(const std::string& variableName)
{
    m_userWatches.erase(
        std::remove(m_userWatches.begin(), m_userWatches.end(), variableName),
        m_userWatches.end()
    );
}

void DebugOverlayPanel::clearWatches()
{
    m_userWatches.clear();
}

void DebugOverlayPanel::setPinned(const std::string& variableName, bool pinned)
{
    for (auto& var : m_variables) {
        if (var.name == variableName) {
            var.isPinned = pinned;
            break;
        }
    }
}

void DebugOverlayPanel::setAnimations(const std::vector<AnimationStateInfo>& animations)
{
    m_animations = animations;
}

void DebugOverlayPanel::setAudioChannels(const std::vector<AudioChannelInfo>& channels)
{
    m_audioChannels = channels;
}

void DebugOverlayPanel::setTimelineTracks(const std::vector<TimelineTrackInfo>& tracks)
{
    m_timelineTracks = tracks;
}

void DebugOverlayPanel::setPlaybackTime(f32 time)
{
    m_playbackTime = time;
}

void DebugOverlayPanel::setCurrentNode(const std::string& nodeId, const std::string& nodeName)
{
    m_currentNodeId = nodeId;
    m_currentNodeName = nodeName;
}

void DebugOverlayPanel::setNextNodes(const std::vector<std::pair<std::string, std::string>>& nodes)
{
    m_nextNodes = nodes;
}

void DebugOverlayPanel::setPerformanceMetrics(const PerformanceMetrics& metrics)
{
    m_metrics = metrics;
}

void DebugOverlayPanel::setOnNavigateToNode(std::function<void(const std::string&)> callback)
{
    m_onNavigateToNode = std::move(callback);
}

void DebugOverlayPanel::setOnNavigateToSource(std::function<void(const std::string&, u32)> callback)
{
    m_onNavigateToSource = std::move(callback);
}

void DebugOverlayPanel::setOnSelectFrame(std::function<void(i32)> callback)
{
    m_onSelectFrame = std::move(callback);
}

// Play mode listener callbacks
void DebugOverlayPanel::onPlayModeStateChanged(PlayModeState oldState, PlayModeState newState)
{
    (void)oldState;
    (void)newState;
    // Could trigger refresh of debug data
}

void DebugOverlayPanel::onPlayModeStarted()
{
    m_hasError = false;
    m_errorMessage.clear();
    open();  // Show panel when play starts
}

void DebugOverlayPanel::onPlayModeStopped()
{
    // Clear runtime data
    m_vmInstructions.clear();
    m_callStack.clear();
    m_animations.clear();
    m_audioChannels.clear();
    m_timelineTracks.clear();
    m_currentNodeId.clear();
    m_currentNodeName.clear();
    m_nextNodes.clear();
}

void DebugOverlayPanel::onPlayModePaused()
{
    // Could highlight that we're paused
}

void DebugOverlayPanel::onBreakpointHit(const Breakpoint& breakpoint)
{
    (void)breakpoint;
    // Highlight the breakpoint location, scroll to it
}

void DebugOverlayPanel::onScriptError(const std::string& error, const std::string& nodeId)
{
    m_hasError = true;
    m_errorMessage = error;
    m_errorNodeId = nodeId;
}

std::vector<MenuItem> DebugOverlayPanel::getMenuItems() const
{
    return {
        {"Clear Error", "", [this]() {
            auto* panel = const_cast<DebugOverlayPanel*>(this);
            panel->m_hasError = false;
            panel->m_errorMessage.clear();
        }, [this]() { return m_hasError; }},
        MenuItem::separator(),
        {"Clear Watches", "", [this]() {
            const_cast<DebugOverlayPanel*>(this)->clearWatches();
        }, nullptr, {}}
    };
}

std::vector<ToolbarItem> DebugOverlayPanel::getToolbarItems() const
{
    return {};  // Toolbar is handled in renderToolbar() directly
}

// Helper function to render a variable entry
void DebugOverlayPanel::renderVariableEntry(const WatchVariable& var)
{
    // Pin button
    bool isPinned = var.isPinned;
    if (widgets::ToolbarToggle(Icons::Pin, &isPinned, "Pin")) {
        const_cast<DebugOverlayPanel*>(this)->setPinned(var.name, isPinned);
    }

    // Variable name
    // ImGui::SameLine();
    // ImGui::Text("%s:", var.name.c_str());

    // Type-colored value
    renderer::Color valueColor = getValueTypeColor(var.type);
    // ImGui::SameLine();
    // ImGui::TextColored(ImVec4(valueColor.r/255.0f, valueColor.g/255.0f, valueColor.b/255.0f, 1.0f),
    //                    "%s", var.value.c_str());

    // Modified indicator
    if (var.isModified) {
        // ImGui::SameLine();
        // ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "(modified)");
    }

    (void)valueColor;
}

} // namespace NovelMind::editor
