# Editor-Engine Interaction

This document describes the data exchange protocols and formats between the NovelMind visual editor and the runtime engine.

## Overview

The editor and engine share data through well-defined formats:

```
+----------------+                      +----------------+
|                |   Project Files      |                |
|    Editor      | ------------------> |    Compiler    |
|                |   (JSON, NMScript)   |                |
+----------------+                      +----------------+
       |                                       |
       | Preview                               | Build
       v                                       v
+----------------+                      +----------------+
|   Embedded     |                      |   Pack Files   |
|   Runtime      |                      |   (.nmres)     |
+----------------+                      +----------------+
                                               |
                                               v
                                        +----------------+
                                        |    Runtime     |
                                        |    Engine      |
                                        +----------------+
```

## Project File Format

### Project Manifest (project.nmproj)

```json
{
  "version": "1.0",
  "name": "My Visual Novel",
  "displayName": "My Visual Novel: Complete Edition",
  "author": "Studio Name",
  "description": "A compelling visual novel experience",
  "settings": {
    "resolution": {
      "width": 1920,
      "height": 1080
    },
    "aspectRatio": "16:9",
    "defaultLanguage": "en",
    "supportedLanguages": ["en", "ja", "ru"],
    "startScene": "prologue"
  },
  "build": {
    "outputName": "MyVisualNovel",
    "version": "1.0.0",
    "platforms": ["windows", "linux", "macos"],
    "encryptAssets": true,
    "compressAssets": true
  }
}
```

### Character Definition (characters/*.json)

```json
{
  "id": "hero",
  "name": "Hero",
  "displayName": {
    "en": "Alex",
    "ja": "アレックス",
    "ru": "Алекс"
  },
  "defaultColor": "#FFCC00",
  "sprites": {
    "idle": "sprites/hero/idle.png",
    "happy": "sprites/hero/happy.png",
    "sad": "sprites/hero/sad.png",
    "angry": "sprites/hero/angry.png"
  },
  "defaultSprite": "idle",
  "voiceProfile": "voice/hero"
}
```

### Scene Definition (scenes/*.json)

```json
{
  "id": "cafe_meeting",
  "name": "Cafe Meeting",
  "background": "backgrounds/cafe_interior.png",
  "music": "music/peaceful_afternoon.ogg",
  "layers": [
    {
      "type": "background",
      "objects": [
        {
          "id": "bg",
          "type": "sprite",
          "resource": "backgrounds/cafe_interior.png",
          "position": { "x": 0, "y": 0 },
          "anchor": { "x": 0, "y": 0 }
        }
      ]
    },
    {
      "type": "characters",
      "objects": []
    },
    {
      "type": "ui",
      "objects": [
        {
          "id": "dialogue_box",
          "type": "ui_panel",
          "template": "dialogue_default"
        }
      ]
    }
  ],
  "entryNode": "node_start",
  "variables": {
    "local_flag": false
  }
}
```

### Story Graph (scenes/*.graph.json)

```json
{
  "sceneId": "cafe_meeting",
  "nodes": [
    {
      "id": "node_start",
      "type": "dialogue",
      "character": "hero",
      "sprite": "idle",
      "text": {
        "en": "Welcome to the cafe!",
        "ja": "カフェへようこそ！"
      },
      "next": "node_choice"
    },
    {
      "id": "node_choice",
      "type": "choice",
      "prompt": {
        "en": "What would you like to order?",
        "ja": "何を注文しますか？"
      },
      "choices": [
        {
          "text": { "en": "Coffee", "ja": "コーヒー" },
          "next": "node_coffee"
        },
        {
          "text": { "en": "Tea", "ja": "紅茶" },
          "next": "node_tea"
        }
      ]
    },
    {
      "id": "node_coffee",
      "type": "dialogue",
      "character": "hero",
      "text": { "en": "Good choice!" },
      "actions": [
        { "type": "set_flag", "flag": "chose_coffee", "value": true }
      ],
      "next": "node_end"
    },
    {
      "id": "node_tea",
      "type": "dialogue",
      "character": "hero",
      "text": { "en": "Excellent taste!" },
      "next": "node_end"
    },
    {
      "id": "node_end",
      "type": "transition",
      "target": "next_scene"
    }
  ]
}
```

### Node Types

| Type | Description |
|------|-------------|
| `dialogue` | Character speaking with text |
| `narration` | Narrator text without character |
| `choice` | Player choice branching |
| `condition` | Conditional branching |
| `transition` | Scene transition |
| `action` | Execute commands without text |
| `wait` | Pause execution |

### Action Types

| Type | Parameters | Description |
|------|------------|-------------|
| `show_character` | character, sprite, position | Show character sprite |
| `hide_character` | character | Hide character |
| `set_sprite` | character, sprite | Change character sprite |
| `move_character` | character, position, duration | Animate character position |
| `set_flag` | flag, value | Set boolean flag |
| `set_variable` | variable, value | Set variable value |
| `play_sound` | sound | Play sound effect |
| `play_music` | music, fadeIn | Play/change music |
| `stop_music` | fadeOut | Stop music |
| `set_background` | background, transition | Change background |
| `screen_effect` | effect, params | Apply screen effect |
| `wait` | duration | Wait for time |
| `call_script` | script | Execute NM Script |

## NM Script Format

### Basic Syntax

```
# Character definitions
character Hero(name="Alex", color="#FFCC00")
character Sidekick(name="Sam", color="#00CCFF")

# Variable declarations
var relationship = 0
flag has_key = false

# Scene definition
scene cafe_intro {
    # Commands
    background "cafe_interior"
    music "peaceful_afternoon" fade 2.0

    show Hero at center
    show Sidekick at right

    # Dialogue
    Hero: "Welcome!"
    Sidekick: "Thanks for having us."

    # Choice
    choice "What would you like?" {
        "Coffee" {
            Hero: "Good choice!"
            set has_coffee = true
        }
        "Tea" {
            Hero: "Excellent!"
        }
    }

    # Conditional
    if has_key {
        Hero: "Let's go!"
        goto secret_room
    } else {
        Hero: "We need to find the key first."
    }
}
```

### Command Reference

```
# Display commands
background <resource> [transition <type> <duration>]
show <character> [<sprite>] at <position> [with <animation>]
hide <character> [with <animation>]
sprite <character> <sprite>
move <character> to <position> [over <duration>]

# Audio commands
music <resource> [fade <duration>] [loop]
sound <resource> [volume <level>]
stop music [fade <duration>]

# Text commands
<character>: "<text>"
<character> (<sprite>): "<text>"
narrate: "<text>"

# Control flow
choice [<prompt>] { ... }
if <condition> { ... } [else { ... }]
goto <scene>
call <script>
return
wait <duration>

# Variables
var <name> = <value>
flag <name> = <true|false>
set <name> = <value>

# Effects
fade <in|out> [<duration>] [<color>]
shake [<intensity>] [<duration>]
flash [<color>] [<duration>]
```

## Compiled Script Format

NM Script is compiled to bytecode for efficient runtime execution.

### Bytecode Header

```
Offset  Size  Description
0x00    4     Magic: "NMSC"
0x04    2     Version
0x06    2     Flags
0x08    4     Instruction count
0x0C    4     Constant pool size
0x10    4     String table size
0x14    4     Symbol table size
```

### Instruction Format

```
Byte 0: Opcode (8 bits)
Bytes 1-4: Operand (32 bits, varies by opcode)
```

### Constant Pool

Contains literals used in the script:
- Integer constants
- Float constants
- String references (offset into string table)

### String Table

Null-terminated UTF-8 strings for:
- Character names
- Dialogue text
- Resource identifiers

## Preview Protocol

The editor contains an embedded runtime for live preview. Communication uses a simple command protocol.

### Preview Commands (Editor -> Runtime)

```json
{"cmd": "load_scene", "scene": "cafe_meeting"}
{"cmd": "goto_node", "node": "node_choice"}
{"cmd": "set_variable", "name": "relationship", "value": 10}
{"cmd": "set_flag", "name": "has_key", "value": true}
{"cmd": "step"}
{"cmd": "pause"}
{"cmd": "resume"}
{"cmd": "reset"}
```

### Preview Events (Runtime -> Editor)

```json
{"event": "scene_loaded", "scene": "cafe_meeting"}
{"event": "node_entered", "node": "node_start", "type": "dialogue"}
{"event": "dialogue", "character": "hero", "text": "Welcome!"}
{"event": "choice_presented", "choices": ["Coffee", "Tea"]}
{"event": "variable_changed", "name": "relationship", "value": 10}
{"event": "error", "message": "Resource not found: missing.png"}
```

## Asset Pipeline

### Supported Source Formats

| Type | Formats | Notes |
|------|---------|-------|
| Images | PNG, JPG, WebP | PNG preferred for transparency |
| Audio | OGG, WAV, MP3 | OGG preferred for music |
| Fonts | TTF, OTF | |
| Scripts | .nms | NM Script source |

### Asset Processing

1. **Images**
   - Convert to optimal format
   - Generate mipmaps (optional)
   - Pack into texture atlases (sprites)
   - Preserve as-is (backgrounds)

2. **Audio**
   - Convert to OGG Vorbis
   - Normalize volume levels
   - Prepare for streaming (music)

3. **Fonts**
   - Rasterize to bitmap fonts
   - Generate SDF for scalable text

4. **Scripts**
   - Parse and validate
   - Compile to bytecode
   - Link references

## Localization Data

### String Table Format (localization/*.json)

```json
{
  "language": "en",
  "strings": {
    "dialogue.cafe_meeting.node_start": "Welcome to the cafe!",
    "dialogue.cafe_meeting.node_choice.prompt": "What would you like?",
    "choice.cafe_meeting.coffee": "Coffee",
    "choice.cafe_meeting.tea": "Tea",
    "ui.save": "Save",
    "ui.load": "Load",
    "ui.options": "Options"
  }
}
```

### String ID Convention

```
dialogue.<scene>.<node>         - Dialogue text
choice.<scene>.<choice_id>      - Choice text
narration.<scene>.<node>        - Narration text
ui.<element>                    - UI text
character.<id>.name             - Character display name
menu.<menu>.<item>              - Menu items
```

## Build Output Structure

```
build/
├── windows/
│   ├── MyVisualNovel.exe       # Executable
│   └── data.nmres              # Resource pack
├── linux/
│   ├── MyVisualNovel           # Executable
│   └── data.nmres              # Resource pack
└── macos/
    └── MyVisualNovel.app/      # Application bundle
        └── Contents/
            ├── MacOS/
            │   └── MyVisualNovel
            └── Resources/
                └── data.nmres
```

## Debug Mode

In debug builds, the engine can:

1. Load unpackaged resources directly from disk
2. Hot-reload modified resources
3. Execute script commands from console
4. Display debug overlays (FPS, memory, etc.)
5. Log detailed execution traces

### Debug Console Commands

```
reload scene              - Reload current scene
reload script <name>      - Reload specific script
goto <scene> [node]       - Jump to scene/node
set <var> <value>         - Set variable
flag <name> <true|false>  - Set flag
dump vars                 - Print all variables
dump flags                - Print all flags
screenshot                - Save screenshot
profile start/stop        - CPU profiling
memory                    - Memory statistics
```
