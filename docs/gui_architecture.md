# NovelMind Editor GUI Architecture

## Overview

This document describes the comprehensive GUI architecture for the NovelMind Editor, rebuilt from scratch using Qt 6 Widgets. The architecture follows a modular, event-driven design inspired by professional editors like Unreal Engine, Unity, and Godot.

## Technology Stack

- **C++20**
- **Qt 6.x**
- **Qt Widgets** (primary UI framework)
- **QDockWidget / QMainWindow** (docking system)
- **QGraphicsView / QGraphicsScene** (graphs, timeline)
- **Qt Model/View** (tables, trees)
- **CMake** (build system)
- **Platforms**: Windows / Linux
- **High-DPI awareness**
- **Dark theme by default**

## Architecture Layers

```
┌─────────────────────────────────────────────────────────────┐
│                      GUI Layer (Qt Widgets)                 │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐           │
│  │SceneView│ │StoryGraph│ │Inspector│ │ Console │  ...     │
│  └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘           │
│       │           │           │           │                 │
├───────┴───────────┴───────────┴───────────┴─────────────────┤
│                    Editor Core Layer                        │
│  ┌─────────┐ ┌─────────┐ ┌──────────┐ ┌────────────────┐   │
│  │Event Bus│ │Selection│ │Undo/Redo │ │Play-In-Editor  │   │
│  │ System  │ │ System  │ │  System  │ │    Bridge      │   │
│  └─────────┘ └─────────┘ └──────────┘ └────────────────┘   │
├─────────────────────────────────────────────────────────────┤
│                    Engine Core (existing)                   │
│  ┌─────────┐ ┌─────────┐ ┌──────────┐ ┌────────────────┐   │
│  │ Scene   │ │Scripting│ │ Assets   │ │   Renderer     │   │
│  │ Graph   │ │  (IR)   │ │  (VFS)   │ │                │   │
│  └─────────┘ └─────────┘ └──────────┘ └────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

## Welcome/Startup Screen

The editor features a modern welcome screen inspired by Unreal Engine and Unity Hub:

**Features:**
- Recent projects list with timestamps and quick access
- Project templates (Blank, Visual Novel, Dating Sim, Mystery, RPG, Horror)
- Quick action buttons (New Project, Open Project, Browse Examples)
- Learning resources panel with documentation links
- Search functionality for projects and templates
- "Don't show again" option with persistent settings
- Command-line `--no-welcome` flag to skip

**Implementation:**
- `NMWelcomeDialog` - Modal dialog shown on startup
- Integrated into `main.cpp` startup flow
- Uses QSettings for persistence
- Unreal-like dark theme styling

## Core Components

### 1. Event Bus (`QtEventBus`)

Central messaging system for loose coupling between components.

```cpp
// Event types
enum class EditorEventType {
    SelectionChanged,
    PropertyChanged,
    GraphModified,
    ProjectLoaded,
    UndoRedoPerformed,
    PlayModeChanged,
    // ...
};

// Usage example
QtEventBus::instance().publish(SelectionChangedEvent{selectedIds});
QtEventBus::instance().subscribe<SelectionChangedEvent>([](const auto& e) {
    // Handle event
});
```

### 2. Selection System (`QtSelectionManager`)

Centralized selection management across all panels.

- Tracks selected objects (scene objects, graph nodes, timeline items, assets)
- Supports multi-selection
- Notifies listeners via Event Bus
- Provides selection history for navigation

### 3. Undo/Redo Command System (`NMUndoManager`)

Centralized undo/redo management using Qt's `QUndoStack`.

**Features:**
- Global undo/redo stack for all editor operations
- Command pattern for reversible actions
- Command merging for smooth operations (e.g., continuous transforms)
- Undo limit configuration (default: 100 operations)
- Clean state tracking for unsaved changes
- Macro support for grouping operations
- Integration with Main Window menu/toolbar
- Dynamic menu text updates

**Base Command Classes:**
- `PropertyChangeCommand` - Property value modifications
- `AddObjectCommand` / `DeleteObjectCommand` - Object lifecycle
- `TransformObjectCommand` - Position/rotation/scale (with merging)
- `CreateGraphNodeCommand` / `DeleteGraphNodeCommand` - Story graph nodes
- `ConnectGraphNodesCommand` - Node connections

```cpp
class MoveNodeCommand : public QUndoCommand {
public:
    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand* other) override;
    int id() const override { return 1; }
};

// Usage
NMUndoManager::instance().pushCommand(new PropertyChangeCommand(...));
```

### 4. Play-In-Editor Bridge

Manages runtime embedding for previewing visual novels in the editor.

## Phased Implementation Roadmap

### Phase 0 - Foundation

**Goals:**
- Application skeleton with Qt6
- Main window with docking
- Theme/Style system (Unreal-like dark theme)
- Event Bus (basic)
- Selection System (basic)

**Key Classes:**
- `NMMainWindow : QMainWindow`
- `NMDockManager`
- `NMStyleManager`
- `QtEventBus`
- `QtSelectionManager`

**Qt Components:**
- `QMainWindow`
- `QDockWidget`
- `QApplication`
- Qt Style Sheets (QSS)

**Definition of Done:**
- [x] Editor launches with empty main window
- [x] Docking framework functional
- [x] Dark theme applied
- [x] Event Bus can publish/subscribe events
- [x] Selection System tracks basic selection
- [x] Welcome/Startup screen with recent projects and templates

### Phase 1 - Core Panels (Read-Only)

**Goals:**
- SceneView panel (display only)
- StoryGraph panel (display only)
- Inspector panel (read-only properties)
- Console panel (log display)

**Key Classes:**
- `NMSceneViewPanel : NMDockPanel`
- `NMStoryGraphPanel : NMDockPanel`
- `NMInspectorPanel : NMDockPanel`
- `NMConsolePanel : NMDockPanel`

**Qt Components:**
- `QGraphicsView` / `QGraphicsScene`
- `QTreeView` / `QTreeWidget`
- `QListWidget`
- `QTextEdit`

**Definition of Done:**
- [x] SceneView displays scene objects (no interaction)
- [x] StoryGraph displays nodes and connections (no editing)
- [x] Inspector shows properties of selected object
- [x] Console displays log messages
- [x] Asset Browser with tree/list split view
- [x] Hierarchy panel with expand/collapse

### Phase 2 - Editable Core

**Goals:**
- Object selection in all panels
- Inspector property editing
- Undo/Redo system
- Basic drag-and-drop
- Asset Browser (basic)

**Key Classes:**
- `NMAssetBrowserPanel : NMDockPanel`
- `NMPropertyEditor`
- `NMUndoManager`
- Various `QUndoCommand` subclasses

**Qt Components:**
- `QUndoStack`
- `QMimeData` (drag-and-drop)
- Custom property widgets

**Definition of Done:**
- [x] Undo/Redo system with QUndoStack
- [x] Command pattern for all editor operations
- [x] Undo/Redo integrated with Main Window
- [ ] Click to select objects (framework ready)
- [ ] Multi-select with Ctrl+Click (framework ready)
- [ ] Inspector edits properties (Phase 2.2 - future work)
- [x] Assets can be browsed

### Phase 3 - Advanced Editors

**Goals:**
- StoryGraph node editing
- Timeline Editor
- Curve Editor
- Hierarchy panel

**Key Classes:**
- `NMTimelinePanel : NMDockPanel`
- `NMCurveEditorPanel : NMDockPanel`
- `NMHierarchyPanel : NMDockPanel`
- `NMGraphNodeItem : QGraphicsItem`
- `NMGraphConnectionItem : QGraphicsItem`

**Qt Components:**
- Custom `QGraphicsItem` subclasses
- `QTimeLine` (for animations)
- Custom painting

**Definition of Done:**
- [x] Timeline Editor with multiple tracks
- [x] Timeline playback controls and frame scrubbing
- [x] Keyframe visualization
- [x] Curve Editor with interpolation types
- [x] Grid visualization and curve path rendering
- [ ] Create/delete/connect nodes in StoryGraph (Phase 3.3 - future work)
- [ ] Hierarchy drag-drop reordering (Phase 3.3 - future work)

### Phase 4 - Production Tools

**Goals:**
- Voice Manager
- Localization Manager
- Diagnostics panel
- Build Settings

**Key Classes:**
- `NMVoiceManagerPanel : NMDockPanel`
- `NMLocalizationPanel : NMDockPanel`
- `NMDiagnosticsPanel : NMDockPanel`
- `NMBuildSettingsPanel : NMDockPanel`

**Definition of Done:**
- [x] Voice Manager panel with import/playback controls
- [x] Voice file list and preview
- [x] Localization Manager panel with language selector
- [x] String table editor for translations
- [x] Diagnostics panel with error/warning display
- [x] Diagnostics filtering by type
- [x] Build Settings panel with platform selector
- [x] Build configuration options and output settings

### Phase 5 - Play-In-Editor

**Goals:**
- Runtime embedding
- Debug overlay
- Breakpoints
- Live variable inspection

**Key Classes:**
- `NMPlayModeController` - Central play/pause/stop coordinator
- `NMPlayToolbarPanel` - Playback control UI
- `NMDebugOverlayPanel` - Runtime variable inspection
- Breakpoint integration in `NMGraphNodeItem`

**Implementation Status (Phase 5.0 - Mock Runtime): ✅ COMPLETE**
- [x] Play Mode Controller with state machine (Stopped/Playing/Paused)
- [x] Mock runtime simulation (1 node/second, demo data)
- [x] Play toolbar panel with Play/Pause/Stop/Step buttons
- [x] Keyboard shortcuts (F5=Play, F6=Pause, F7=Stop, F10=Step)
- [x] Debug overlay panel with 6 tabs (Variables/CallStack/Instruction/Animations/Audio/Performance)
- [x] Variable editing when paused (double-click to edit)
- [x] Breakpoint management system (add/remove/toggle/persist)
- [x] Qt signals/slots for all state changes
- [x] **Visual breakpoint indicators in StoryGraph (red circle with 3D effect)**
- [x] **Current node highlighting during playback (pulsing green glow + execution arrow)**
- [x] **Context menu for breakpoint toggle (right-click on node)**
- [x] **Integration into main window (panels docked and registered)**
- [x] **Auto-centering on currently executing node**

**Definition of Done (Phase 5.0): ✅ ALL COMPLETE**
- [x] Can play through demo nodes in mock runtime
- [x] Debug overlay shows runtime variables and call stack
- [x] Can add/remove breakpoints programmatically
- [x] Can toggle breakpoints via UI (context menu)
- [x] Live variables visible and editable during pause
- [x] Visual feedback for breakpoints and execution state
- [x] Professional keyboard shortcuts matching industry tools

**Future Work (Phase 5.1+):**
- [ ] Real runtime integration (replace mock with ScriptVM)
- [ ] Runtime scene rendering in SceneView
- [ ] Timeline playback synchronization
- [ ] Hot reload during play mode
- [ ] Advanced debugging (conditional breakpoints, watch expressions)

## Panel Specifications

### Main Window + Docking

The main window uses `QMainWindow` with `QDockWidget` for all panels. Layout is saved/restored via `QSettings`.

```cpp
class NMMainWindow : public QMainWindow {
    Q_OBJECT
public:
    void saveLayout();
    void restoreLayout();
    void resetToDefaultLayout();

private:
    QMenuBar* m_menuBar;
    QToolBar* m_mainToolBar;
    QStatusBar* m_statusBar;
    // Dock panels
    NMSceneViewPanel* m_sceneView;
    NMStoryGraphPanel* m_storyGraph;
    // ...
};
```

### SceneView Panel

Displays the visual novel scene with objects, backgrounds, and characters.

- Uses `QGraphicsView` with custom `QGraphicsScene`
- Supports pan (middle-mouse) and zoom (scroll wheel)
- Grid overlay (optional)
- Object selection rectangles
- Transform gizmos (Phase 2+)

### StoryGraph Panel

Node-based visual script editor.

- Uses `QGraphicsView` with custom node items
- Custom `QGraphicsItem` for nodes
- Bezier curve connections
- Mini-map (optional)
- Node palette for creating new nodes

### Inspector Panel

Property editor for selected objects.

- Uses `QScrollArea` with property widgets
- Property groups (collapsible)
- Various editors: text, number, color, dropdown, file picker
- Multi-object editing (common properties)

### Console Panel

Log output and command input.

- Uses `QTextEdit` (read-only) for log display
- Filter buttons (Info, Warning, Error)
- Clear button
- Auto-scroll option
- Search/filter text input

## Style Guide

### Color Palette (Unreal-like Dark Theme)

```css
/* Background colors */
--bg-darkest:     #1a1a1a;   /* Main background */
--bg-dark:        #232323;   /* Panel backgrounds */
--bg-medium:      #2d2d2d;   /* Widget backgrounds */
--bg-light:       #383838;   /* Hover states */

/* Text colors */
--text-primary:   #e0e0e0;   /* Primary text */
--text-secondary: #a0a0a0;   /* Secondary text */
--text-disabled:  #606060;   /* Disabled text */

/* Accent colors */
--accent-primary: #0078d4;   /* Selection, focus */
--accent-hover:   #1a88e0;   /* Hover state */
--accent-active:  #006cbd;   /* Active state */

/* Status colors */
--error:          #f44336;
--warning:        #ff9800;
--success:        #4caf50;
--info:           #2196f3;

/* Border colors */
--border-dark:    #1a1a1a;
--border-light:   #404040;
```

### Typography

- **Font Family**: Segoe UI (Windows), Ubuntu (Linux), System default
- **Base Size**: 9pt (11px at 96 DPI)
- **Headers**: 10-12pt, semi-bold
- **Monospace**: Consolas, Ubuntu Mono (for code/console)

### Spacing

- **Panel Padding**: 4px
- **Widget Spacing**: 4px
- **Group Spacing**: 8px
- **Section Spacing**: 16px

### Panel Behavior

- Panels can be docked to any edge
- Panels can be tabbed with other panels
- Panels can float as separate windows
- Panels remember their size and position
- Double-click title bar to float/dock

### Widget States

All interactive widgets have distinct visual states:

- **Normal**: Default appearance
- **Hover**: Slightly lighter background
- **Pressed/Active**: Accent color highlight
- **Focused**: Accent border
- **Disabled**: Reduced opacity, gray text
- **Selected**: Accent background

### Icons

- Use SVG icons for scalability
- Icon size: 16x16 (toolbar), 24x24 (large actions)
- Monochrome icons with accent color for active state

## File Structure

```
editor/
├── include/NovelMind/editor/
│   ├── qt/                         # Qt-specific implementations
│   │   ├── nm_main_window.hpp
│   │   ├── nm_dock_panel.hpp
│   │   ├── nm_style_manager.hpp
│   │   ├── qt_event_bus.hpp
│   │   ├── qt_selection_manager.hpp
│   │   ├── panels/
│   │   │   ├── nm_scene_view_panel.hpp
│   │   │   ├── nm_story_graph_panel.hpp
│   │   │   ├── nm_inspector_panel.hpp
│   │   │   ├── nm_console_panel.hpp
│   │   │   ├── nm_asset_browser_panel.hpp
│   │   │   ├── nm_hierarchy_panel.hpp
│   │   │   ├── nm_timeline_panel.hpp
│   │   │   ├── nm_curve_editor_panel.hpp
│   │   │   ├── nm_voice_manager_panel.hpp
│   │   │   ├── nm_localization_panel.hpp
│   │   │   ├── nm_diagnostics_panel.hpp
│   │   │   └── nm_build_settings_panel.hpp
│   │   └── widgets/
│   │       ├── nm_property_editor.hpp
│   │       ├── nm_graph_node_item.hpp
│   │       └── nm_timeline_track.hpp
│   └── editor_app.hpp              # Main application (Qt version)
├── src/
│   ├── qt/
│   │   ├── nm_main_window.cpp
│   │   ├── nm_dock_panel.cpp
│   │   ├── nm_style_manager.cpp
│   │   ├── qt_event_bus.cpp
│   │   ├── qt_selection_manager.cpp
│   │   ├── panels/
│   │   │   └── ... (panel implementations)
│   │   └── widgets/
│   │       └── ... (widget implementations)
│   └── main.cpp                    # Qt application entry point
└── resources/
    ├── styles/
    │   └── dark_theme.qss
    └── icons/
        └── ... (SVG icons)
```

## Scalability Principles

1. **Panel Registration**: Panels are registered with a factory, allowing new panels to be added without modifying core code.

2. **Event-Driven Communication**: All panels communicate through Event Bus, preventing tight coupling.

3. **Command Pattern**: All modifications go through the Undo/Redo system as commands.

4. **Settings Persistence**: All user preferences saved via `QSettings`.

5. **Plugin Architecture** (Future): Support for loadable panel plugins.

## Integration with Engine Core

The GUI layer interfaces with the existing engine core through:

1. **SceneGraph**: Direct access to scene objects for display and manipulation
2. **VisualGraph (IR)**: Story graph node data
3. **VFS**: Asset loading and management
4. **Logger**: Console output
5. **PropertySystem**: Object property reflection

The existing backend implementations (Event Bus, Selection System, etc.) are preserved and wrapped with Qt-compatible interfaces.
