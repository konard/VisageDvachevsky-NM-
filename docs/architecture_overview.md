# Architecture Overview

This document describes the high-level architecture of the NovelMind engine and its components.

## System Components

NovelMind consists of three main components:

```
+-------------------+     +-------------------+     +-------------------+
|   Visual Editor   |---->|     Compiler      |---->|  Runtime Engine   |
+-------------------+     +-------------------+     +-------------------+
        |                         |                         |
        v                         v                         v
   Project Files            Pack Files (.nmres)       Game Executable
   (JSON/NMScript)          + Compiled Scripts
```

### 1. Runtime Engine (`engine_core/`)

The runtime engine is the core component that executes visual novels at runtime. It is designed to be lightweight and fast.

#### Module Architecture

```
+------------------------------------------------------------------+
|                        Application Layer                          |
+------------------------------------------------------------------+
         |              |              |              |
         v              v              v              v
+----------------+  +--------+  +-----------+  +-------------+
| Scene Manager  |  | Story  |  | Save/Load |  | Localization|
|                |  | Engine |  |  System   |  |   Manager   |
+----------------+  +--------+  +-----------+  +-------------+
         |              |              |              |
         v              v              v              v
+------------------------------------------------------------------+
|                        Core Services                              |
+------------------------------------------------------------------+
|  +-------------+  +----------------+  +------------------------+  |
|  |  Renderer   |  |  Audio System  |  |  Resource Manager      |  |
|  +-------------+  +----------------+  +------------------------+  |
|  +-------------+  +----------------+  +------------------------+  |
|  |   Input     |  |   Scripting    |  |  Virtual File System   |  |
|  +-------------+  +----------------+  +------------------------+  |
+------------------------------------------------------------------+
         |              |              |              |
         v              v              v              v
+------------------------------------------------------------------+
|                      Platform Abstraction                         |
|  +--------+  +--------+  +--------+  +--------+  +--------+      |
|  | Window |  |  Time  |  |  File  |  | Events |  | Thread |      |
|  +--------+  +--------+  +--------+  +--------+  +--------+      |
+------------------------------------------------------------------+
```

#### Core Modules

| Module | Namespace | Description |
|--------|-----------|-------------|
| Platform | `nm::platform` | Window management, input, timing, file I/O |
| Renderer | `nm::renderer` | 2D sprite rendering, text, effects |
| VFS | `nm::vfs` | Virtual file system with encryption support |
| Resource Manager | `nm::resource` | Asset loading, caching, lifecycle |
| Scripting | `nm::scripting` | NM Script interpreter/VM |
| Scene | `nm::scene` | Scene graph, layers, objects |
| Audio | `nm::audio` | Sound and music playback |
| Input | `nm::input` | Input mapping and event handling |
| Save | `nm::save` | Game state serialization |

### 2. Visual Editor (`editor/`)

The visual editor is a desktop application for creating visual novel projects.

#### Editor Components

- **Project Dashboard** - Project management, settings
- **Scene Editor** - Visual scene composition (WYSIWYG)
- **Story Flow Editor** - Node-based narrative flow design
- **Script Editor** - NM Script code editing with syntax highlighting
- **Localization Editor** - Translation management
- **Build System** - Project compilation interface

### 3. Compiler (`compiler/`)

The compiler transforms editor projects into runtime-ready packages.

#### Compilation Pipeline

```
Source Assets          Intermediate           Final Output
-------------          ------------           ------------
Images (.png)    --->  Texture Atlas    --->  Encrypted Pack
Audio (.ogg)     --->  Audio Index      --->  (.nmres)
Scripts (.nms)   --->  Bytecode         --->
Scenes (.json)   --->  Binary Scene     --->
```

## Data Flow

### Project Data Model

```
Project
├── Metadata (name, version, settings)
├── Characters[]
│   ├── id, name, display_name
│   ├── sprites[]
│   └── default_color
├── Scenes[]
│   ├── id, name
│   ├── layers[]
│   │   └── objects[]
│   └── entry_node
├── Story Graph
│   └── nodes[]
│       ├── DialogueNode
│       ├── ChoiceNode
│       ├── ConditionNode
│       └── TransitionNode
├── Variables[]
│   ├── Flags (bool)
│   └── Counters (int)
├── Localization
│   └── languages[]
│       └── strings{}
└── Assets
    ├── textures/
    ├── audio/
    └── fonts/
```

### Runtime Data Flow

1. **Initialization**
   - Platform layer initializes window, input, timer
   - VFS mounts encrypted pack files
   - Resource manager preloads essential assets
   - Story engine loads initial scene

2. **Main Loop**
   - Process input events
   - Update story state
   - Update animations/effects
   - Render scene layers
   - Present frame

3. **Story Execution**
   - Fetch current node from story graph
   - Execute node commands via scripting engine
   - Wait for player input (dialogue advance, choice selection)
   - Transition to next node

## Dependency Graph

```
                    Application
                         |
         +-------+-------+-------+-------+
         |       |       |       |       |
         v       v       v       v       v
      Scene   Story    Save   Local   Audio
         |       |       |       |       |
         +-------+---+---+-------+       |
                     |                   |
                     v                   |
              Resource Manager <---------+
                     |
                     v
            Virtual File System
                     |
                     v
              Platform Layer
```

## Threading Model

The engine uses a single-threaded model for simplicity:

- **Main Thread**: Game loop, rendering, input, audio commands
- **Audio Thread** (optional): Streaming audio playback (managed by audio backend)

Future considerations:
- Resource loading on background thread
- Script execution on dedicated thread

## Memory Management

- **RAII principles** throughout the codebase
- **Smart pointers** for ownership semantics
- **Resource handles** for managed resources
- **Pool allocators** for frequently created/destroyed objects

## Error Handling

- **Expected/Result types** for recoverable errors
- **Exceptions disabled** - explicit error propagation
- **Logging system** for debugging and diagnostics
- **Assertions** for development-time checks
