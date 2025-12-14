# Phase 5 Design: Play-In-Editor System

## Overview

Phase 5 implements the Play-In-Editor (PIE) system, allowing developers to preview and debug visual novels directly within the editor. This phase integrates the NovelMind runtime engine with the Qt GUI, providing real-time debugging capabilities.

## Architecture

```
┌──────────────────────────────────────────────────────────┐
│                   Qt GUI Layer                           │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐   │
│  │ Play Toolbar │  │ Debug Overlay│  │  StoryGraph  │   │
│  │  (PIE Panel) │  │    Panel     │  │  (w/ BPs)    │   │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘   │
│         │                 │                  │           │
├─────────┴─────────────────┴──────────────────┴───────────┤
│               Play-In-Editor Controller                  │
│  ┌────────────────────────────────────────────────────┐  │
│  │  NMPlayModeController (Singleton)                  │  │
│  │  - playMode: Stopped / Playing / Paused           │  │
│  │  - runtimeThread: Separate thread for engine      │  │
│  │  - breakpointMgr: Manages breakpoints             │  │
│  │  - variableWatch: Live variable inspection        │  │
│  └────────────────────────────────────────────────────┘  │
├──────────────────────────────────────────────────────────┤
│               NovelMind Runtime Engine                   │
│  ┌────────────────────────────────────────────────────┐  │
│  │  - Scene Manager                                   │  │
│  │  - Script VM (IR executor)                        │  │
│  │  - Asset Loader (VFS)                             │  │
│  │  - Animation System                               │  │
│  │  - Audio Manager                                  │  │
│  └────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────┘
```

## Components

### 1. NMPlayModeController

**Responsibility**: Central coordinator for Play-In-Editor mode

**Key Methods:**
```cpp
class NMPlayModeController : public QObject {
    Q_OBJECT
public:
    enum PlayMode { Stopped, Playing, Paused };

    static NMPlayModeController& instance();

    // Playback control
    void play();
    void pause();
    void stop();
    void stepForward();  // Execute one instruction

    // State queries
    PlayMode playMode() const;
    bool isPlaying() const;
    bool isPaused() const;

    // Breakpoint management
    void toggleBreakpoint(const QString& nodeId);
    bool hasBreakpoint(const QString& nodeId) const;
    void clearAllBreakpoints();

    // Variable inspection
    QVariantMap getCurrentVariables() const;
    void setVariable(const QString& name, const QVariant& value);

signals:
    void playModeChanged(PlayMode mode);
    void breakpointHit(const QString& nodeId);
    void variablesChanged(const QVariantMap& variables);
    void currentNodeChanged(const QString& nodeId);

private:
    PlayMode m_playMode = Stopped;
    QSet<QString> m_breakpoints;
    QString m_currentNodeId;
    QVariantMap m_variables;

    // Runtime integration (to be implemented)
    // std::unique_ptr<RuntimeBridge> m_runtime;
};
```

### 2. NMPlayToolbar Panel

**Responsibility**: Playback controls in main toolbar

**Features:**
- Play button (▶️) - Start execution
- Pause button (⏸️) - Pause at next instruction
- Stop button (⏹️) - Terminate and reset
- Step Forward button (⏭️) - Execute one instruction
- Play mode indicator (status label)
- Frame counter / time display

**UI Layout:**
```
┌────────────────────────────────────────────────┐
│ [▶️ Play] [⏸️ Pause] [⏹️ Stop] [⏭️ Step] │ Stopped │
└────────────────────────────────────────────────┘
```

### 3. NMDebugOverlayPanel

**Responsibility**: Runtime variable and state inspection

**Features:**
- **Variables Tab**: Live VM variables with editable values
- **Call Stack Tab**: Current execution stack
- **Animation Tab**: Active animations with progress bars
- **Audio Tab**: Playing audio channels with volume meters
- **Performance Tab**: Frame time, memory usage

**UI Layout (Variables Tab):**
```
┌─────────────────────────────────────────┐
│ Variables                         🔍    │
├─────────────────────────────────────────┤
│ 📁 Global Variables                     │
│   ├─ playerName: "Alice"          [✏️] │
│   ├─ affection: 75                [✏️] │
│   └─ chapter: 3                   [✏️] │
│ 📁 Local Variables                      │
│   ├─ tempChoice: "Option A"       [✏️] │
│   └─ loopCounter: 5               [✏️] │
├─────────────────────────────────────────┤
│ Call Stack                              │
│   ├─ scene_001::dialogue_loop    (42)  │
│   └─ main::start                  (15)  │
├─────────────────────────────────────────┤
│ Active Animations (2)                   │
│   ├─ character_fade ▓▓▓▓▓▓░░░ 65%      │
│   └─ bg_transition  ▓▓▓▓▓▓▓▓▓ 95%      │
└─────────────────────────────────────────┘
```

### 4. Breakpoint System Integration

**Responsibility**: Pause execution at specific graph nodes

**Implementation:**
- Visual breakpoint indicator in StoryGraph panel (red dot 🔴)
- Click gutter to toggle breakpoints
- Breakpoints persist in project settings
- Breakpoint hit triggers `breakpointHit()` signal
- Current node highlighted with yellow border

**StoryGraph Integration:**
```cpp
class NMStoryGraphPanel : public NMDockPanel {
    // ... existing code ...

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void drawBreakpointGutter(QPainter& painter);
    void handleBreakpointClick(const QPoint& pos);

    QSet<QString> m_nodeBreakpoints;  // Synced with controller
    QString m_currentExecutingNode;    // Highlighted during play
};
```

### 5. Runtime Thread Management

**Responsibility**: Run engine in separate thread to avoid blocking UI

**Design:**
```cpp
class RuntimeWorker : public QObject {
    Q_OBJECT
public:
    RuntimeWorker();

public slots:
    void startRuntime(const QString& projectPath);
    void pauseRuntime();
    void stopRuntime();
    void stepInstruction();

signals:
    void runtimeStarted();
    void runtimePaused(const QString& nodeId);
    void runtimeStopped();
    void variableUpdate(const QVariantMap& vars);
    void nodeExecuted(const QString& nodeId);

private:
    // void executeInstructionLoop();
    // std::unique_ptr<ScriptVM> m_vm;
};

// In NMPlayModeController:
void NMPlayModeController::play() {
    if (!m_workerThread) {
        m_workerThread = new QThread(this);
        m_worker = new RuntimeWorker;
        m_worker->moveToThread(m_workerThread);

        connect(m_worker, &RuntimeWorker::runtimePaused,
                this, &NMPlayModeController::onRuntimePaused);
        // ... more connections

        m_workerThread->start();
    }

    emit startRuntime(currentProjectPath());
    m_playMode = Playing;
    emit playModeChanged(Playing);
}
```

## Implementation Plan

### Step 1: Controller Foundation
- Implement `NMPlayModeController` singleton
- Add play/pause/stop state machine
- Emit signals for state changes
- Add to main window toolbar

### Step 2: Play Toolbar Panel
- Create `NMPlayToolbarPanel` with buttons
- Connect buttons to controller methods
- Update button states based on play mode
- Add keyboard shortcuts (F5 = Play, Shift+F5 = Stop)

### Step 3: Breakpoint System
- Add breakpoint toggle in `NMStoryGraphPanel`
- Visual breakpoint indicators (red dot)
- Sync breakpoints with controller
- Persist breakpoints in project settings

### Step 4: Debug Overlay Panel
- Create `NMDebugOverlayPanel` with tabs
- Variables tree view with edit capability
- Connect to controller variable signals
- Add refresh button and auto-update option

### Step 5: Runtime Integration (Stub for now)
- Create runtime bridge interface
- Implement mock VM for testing
- Add thread management for non-blocking execution
- Connect runtime events to controller signals

### Step 6: Current Node Highlighting
- Highlight current node in StoryGraph during play
- Scroll to active node automatically
- Show execution path with faded connection lines
- Timeline playback sync (if applicable)

## User Workflows

### Workflow 1: Basic Playback
1. User clicks "Play" button (or presses F5)
2. Controller enters Playing mode
3. Runtime starts executing from entry node
4. Current node highlighted in StoryGraph
5. Variables updated in Debug Overlay
6. User clicks "Stop" to end

### Workflow 2: Debugging with Breakpoints
1. User right-clicks node in StoryGraph → "Toggle Breakpoint"
2. Red dot appears on node
3. User clicks "Play"
4. Runtime executes until breakpoint node
5. Controller enters Paused mode
6. User inspects variables in Debug Overlay
7. User clicks "Step Forward" to continue step-by-step

### Workflow 3: Live Variable Editing
1. Runtime is paused at breakpoint
2. User opens Debug Overlay → Variables tab
3. User double-clicks variable value → edit dialog
4. User changes "affection" from 50 to 100
5. Controller applies change to runtime
6. User clicks "Play" to continue with new value

## Event Flow

```
User Action → Controller → Runtime → Controller → UI Update

Example: Play Button Click
├─ User: Click Play button
├─ NMPlayToolbarPanel: emit playClicked()
├─ NMPlayModeController: play()
│  ├─ Start RuntimeWorker thread
│  ├─ m_playMode = Playing
│  └─ emit playModeChanged(Playing)
├─ RuntimeWorker: executeInstructionLoop()
│  ├─ VM executes node instructions
│  ├─ emit nodeExecuted(nodeId)
│  └─ emit variableUpdate(vars)
├─ NMPlayModeController: Receive signals
│  ├─ Update m_currentNodeId
│  ├─ Update m_variables
│  ├─ emit currentNodeChanged(nodeId)
│  └─ emit variablesChanged(vars)
└─ UI Panels: Update visuals
   ├─ NMStoryGraphPanel: Highlight current node
   ├─ NMDebugOverlayPanel: Refresh variable tree
   └─ NMPlayToolbarPanel: Update status label
```

## Integration with Existing Systems

### Event Bus Integration
```cpp
// Publish events from controller
QtEventBus::instance().publish(PlayModeChangedEvent{m_playMode});
QtEventBus::instance().publish(BreakpointHitEvent{nodeId});

// Panels subscribe to events
QtEventBus::instance().subscribe<PlayModeChangedEvent>([this](const auto& e) {
    updatePlayModeUI(e.playMode);
});
```

### Undo/Redo Integration
- Variable edits during play mode do NOT go through Undo/Redo
- Only project-level changes (adding breakpoints) are undoable
- Play mode is a separate "debug state" not part of project state

### Selection System Integration
- Current executing node is NOT treated as "selected"
- Separate visual indicator (yellow border vs blue selection)
- User can still select other nodes while playing

## File Structure

```
editor/
├── include/NovelMind/editor/qt/
│   ├── nm_play_mode_controller.hpp      # NEW
│   ├── nm_breakpoint_manager.hpp        # NEW
│   └── panels/
│       ├── nm_play_toolbar_panel.hpp    # NEW
│       └── nm_debug_overlay_panel.hpp   # NEW
├── src/qt/
│   ├── nm_play_mode_controller.cpp      # NEW
│   ├── nm_breakpoint_manager.cpp        # NEW
│   └── panels/
│       ├── nm_play_toolbar_panel.cpp    # NEW
│       └── nm_debug_overlay_panel.cpp   # NEW
```

## Testing Strategy

### Unit Tests
- Controller state transitions (Stopped → Playing → Paused → Stopped)
- Breakpoint add/remove/toggle
- Variable get/set operations

### Integration Tests
- Signal/slot connections
- Multi-threaded runtime execution
- UI updates on state changes

### Manual Testing
- Keyboard shortcut responsiveness
- Breakpoint visual indicators
- Variable editing during pause
- Thread safety (no UI freezing)

## Phase 5 Definition of Done

- [ ] `NMPlayModeController` implemented with state machine
- [ ] Play toolbar with Play/Pause/Stop/Step buttons
- [ ] Breakpoint system in StoryGraph panel
- [ ] Debug Overlay panel with variable tree
- [ ] Current node highlighting during playback
- [ ] Runtime thread management (stub implementation)
- [ ] F5 keyboard shortcut for Play
- [ ] All signals/slots connected
- [ ] No UI blocking during playback
- [ ] Breakpoints persist in project settings
- [ ] CI passing on all platforms

## Future Enhancements (Post-Phase 5)

- **Advanced Debugging**: Conditional breakpoints, watch expressions
- **Performance Profiling**: CPU/memory usage per node
- **Hot Reload**: Edit scripts during play mode
- **Recording**: Record playthrough for testing
- **Network Debugging**: Inspect networked visual novel state
- **Scripting Console**: Execute arbitrary commands during play

## Notes

**Runtime Integration**: For Phase 5.0, we'll implement a **mock runtime** that simulates node execution without full engine integration. This allows us to validate the GUI architecture before the complex engine integration.

**Mock Runtime Behavior**:
- Step through demo StoryGraph nodes at 1 node/second
- Generate fake variables (playerName, affection, chapter)
- Respect breakpoints (pause when hit)
- Thread-safe communication with GUI

**Real Runtime Integration** (Phase 5.1+):
- Replace mock with actual ScriptVM
- Integrate scene rendering in SceneView
- Connect Timeline playback
- Full audio/animation system

---

**Status**: Ready for implementation
**Dependencies**: None (builds on Phases 0-4)
**Estimated Complexity**: Medium-High
**Priority**: HIGH - Core editor functionality
