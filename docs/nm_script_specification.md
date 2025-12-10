# NM Script 1.0 Language Specification

## Overview

NM Script is a domain-specific language designed for visual novel development in the NovelMind engine. It provides a clean, intuitive syntax for defining characters, scenes, dialogues, choices, and game logic.

## Version Information

- **Version**: 1.0
- **Status**: Draft
- **Last Updated**: 2025-12-10

## Table of Contents

1. [Lexical Structure](#lexical-structure)
2. [Data Types](#data-types)
3. [Character Declarations](#character-declarations)
4. [Scene Declarations](#scene-declarations)
5. [Statements](#statements)
6. [Expressions](#expressions)
7. [Control Flow](#control-flow)
8. [Built-in Commands](#built-in-commands)
9. [Comments](#comments)
10. [Reserved Keywords](#reserved-keywords)
11. [Grammar (EBNF)](#grammar-ebnf)
12. [Semantic Rules](#semantic-rules)
13. [Compatibility Policy](#compatibility-policy)

---

## 1. Lexical Structure

### Identifiers

Identifiers must start with a letter (a-z, A-Z) or underscore (_), followed by any combination of letters, digits (0-9), or underscores.

```
identifier ::= [a-zA-Z_][a-zA-Z0-9_]*
```

Valid examples: `hero`, `_private`, `character1`, `MainScene`

### String Literals

String literals are enclosed in double quotes. Escape sequences are supported:

- `\n` - newline
- `\t` - tab
- `\\` - backslash
- `\"` - double quote
- `\{` - literal opening brace (escape rich text command)

```nms
"Hello, World!"
"Line 1\nLine 2"
"He said \"Hello\""
```

### Number Literals

Numbers can be integers or floating-point:

```nms
42          // integer
3.14        // float
-17         // negative integer
0.5         // decimal less than 1
```

### Boolean Literals

```nms
true
false
```

---

## 2. Data Types

NM Script supports the following data types:

| Type | Description | Example |
|------|-------------|---------|
| `string` | Text data | `"Hello"` |
| `int` | Integer numbers | `42` |
| `float` | Floating-point numbers | `3.14` |
| `bool` | Boolean values | `true`, `false` |
| `void` | No value (for commands) | - |

Type coercion is automatic in most cases:
- `int` to `float`: implicit
- `float` to `int`: implicit (truncates)
- Any to `string`: implicit in string contexts
- Any to `bool`: 0, 0.0, "", and false are falsy; all else is truthy

---

## 3. Character Declarations

Characters must be declared before use. The declaration defines the character's identifier, display name, and optional styling.

### Syntax

```nms
character <id>(<properties>)
```

### Properties

| Property | Type | Required | Description |
|----------|------|----------|-------------|
| `name` | string | Yes | Display name shown in dialogue |
| `color` | string | No | Name color (hex: `"#RRGGBB"`) |
| `voice` | string | No | Voice track identifier |

### Examples

```nms
// Basic character
character Hero(name="Alex")

// Character with styling
character Villain(name="Lord Darkness", color="#FF0000")

// Character with voice
character Narrator(name="Narrator", voice="narrator_voice")
```

---

## 4. Scene Declarations

Scenes are the primary organizational unit in NM Script. Each scene contains a sequence of statements.

### Syntax

```nms
scene <id> {
    <statements>
}
```

### Rules

- Scene IDs must be unique within a script
- At least one scene must be defined
- Scenes can reference other scenes via `goto`

### Example

```nms
scene intro {
    show background "bg_city_night"
    show Hero at center

    say Hero "Welcome to the adventure!"

    goto chapter1
}

scene chapter1 {
    // Chapter 1 content
}
```

---

## 5. Statements

### Show Statement

Displays visual elements on screen.

```nms
// Show background
show background "<texture_id>"

// Show character
show <character_id> at <position>
show <character_id> at <position> with <expression>

// Positions: left, center, right, or custom coordinates
show Hero at left
show Hero at center with "happy"
show Hero at (100, 200)
```

### Hide Statement

Removes visual elements from screen.

```nms
hide <character_id>
hide background
```

### Say Statement

Displays dialogue text.

```nms
say <character_id> "<text>"

// Examples
say Hero "Hello!"
say Narrator "The story begins..."
```

Rich text commands can be embedded in dialogue:

| Command | Description | Example |
|---------|-------------|---------|
| `{w=N}` | Wait N seconds | `"Hello...{w=0.5}world"` |
| `{speed=N}` | Set typing speed | `{speed=50}` |
| `{color=#RRGGBB}` | Change text color | `{color=#FF0000}Red` |
| `{/color}` | Reset color | `{/color}` |
| `{shake}` | Shake effect | `{shake}Scary!{/shake}` |
| `{wave}` | Wave effect | `{wave}Hello~{/wave}` |

### Choice Statement

Presents options to the player.

```nms
choice {
    "Option 1" -> <action>
    "Option 2" -> <action>
    "Option 3" if <condition> -> <action>
}
```

Actions can be:
- `goto <scene_id>` - Jump to scene
- Block of statements `{ ... }`

### Example

```nms
say Hero "What should I do?"

choice {
    "Go left" -> goto left_path
    "Go right" -> goto right_path
    "Wait" -> {
        say Hero "I'll wait here..."
        goto wait_scene
    }
    "Secret option" if has_key -> goto secret_path
}
```

### Set Statement

Assigns values to variables or flags.

```nms
// Set variable
set <variable> = <expression>

// Set flag
set flag <flag_name> = <bool>

// Examples
set points = 10
set player_name = "Alex"
set flag visited_shop = true
```

### Wait Statement

Pauses execution.

```nms
wait <seconds>

// Example
wait 1.5
```

### Goto Statement

Jumps to another scene.

```nms
goto <scene_id>
```

### Transition Statement

Applies a visual transition.

```nms
transition <type> <duration>

// Types: fade, dissolve, slide_left, slide_right, slide_up, slide_down
transition fade 0.5
transition dissolve 1.0
```

### Play Statement

Plays audio.

```nms
// Play music (loops by default)
play music "<track_id>"
play music "<track_id>" loop=false

// Play sound effect
play sound "<sound_id>"

// Play voice
play voice "<voice_id>"
```

### Stop Statement

Stops audio.

```nms
stop music
stop music fade=1.0
stop sound
stop voice
```

---

## 6. Expressions

### Operators

#### Arithmetic

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `a + b` |
| `-` | Subtraction | `a - b` |
| `*` | Multiplication | `a * b` |
| `/` | Division | `a / b` |
| `%` | Modulo | `a % b` |

#### Comparison

| Operator | Description | Example |
|----------|-------------|---------|
| `==` | Equal | `a == b` |
| `!=` | Not equal | `a != b` |
| `<` | Less than | `a < b` |
| `<=` | Less or equal | `a <= b` |
| `>` | Greater than | `a > b` |
| `>=` | Greater or equal | `a >= b` |

#### Logical

| Operator | Description | Example |
|----------|-------------|---------|
| `&&` | Logical AND | `a && b` |
| `\|\|` | Logical OR | `a \|\| b` |
| `!` | Logical NOT | `!a` |

### Precedence (highest to lowest)

1. `!` (unary NOT)
2. `*`, `/`, `%`
3. `+`, `-`
4. `<`, `<=`, `>`, `>=`
5. `==`, `!=`
6. `&&`
7. `||`

### Examples

```nms
set total = price * quantity
set can_afford = money >= total
set should_buy = can_afford && wants_item
```

---

## 7. Control Flow

### If Statement

Conditional execution.

```nms
if <condition> {
    <statements>
}

if <condition> {
    <statements>
} else {
    <statements>
}

if <condition> {
    <statements>
} else if <condition> {
    <statements>
} else {
    <statements>
}
```

### Example

```nms
if points >= 100 {
    say Narrator "You've earned the best ending!"
    goto best_ending
} else if points >= 50 {
    say Narrator "You've done well."
    goto good_ending
} else {
    say Narrator "Better luck next time..."
    goto bad_ending
}
```

### Flag Checks

```nms
if flag visited_shop {
    say Shopkeeper "Welcome back!"
}

if !flag has_key {
    say Hero "The door is locked..."
}
```

---

## 8. Built-in Commands

### Screen Effects

```nms
// Screen shake
shake intensity=5 duration=0.5

// Screen flash
flash color="#FFFFFF" duration=0.3

// Fade to color
fade_to color="#000000" duration=1.0
fade_from color="#000000" duration=1.0
```

### Character Animation

```nms
// Move character
move <character_id> to <position> duration=<seconds>

// Scale character
scale <character_id> to <factor> duration=<seconds>

// Rotate character
rotate <character_id> to <angle> duration=<seconds>
```

### UI Commands

```nms
// Show/hide dialogue box
show textbox
hide textbox

// Set text speed
set_speed <chars_per_second>

// Enable/disable skip mode
allow_skip true
allow_skip false
```

---

## 9. Comments

```nms
// Single-line comment

/*
 * Multi-line
 * comment
 */

character Hero(name="Alex")  // Inline comment
```

---

## 10. Reserved Keywords

The following identifiers are reserved and cannot be used as variable, character, or scene names:

```
and         character   choice      else        false
flag        goto        hide        if          music
not         or          play        say         scene
set         show        sound       stop        then
transition  true        voice       wait        with
```

---

## 11. Grammar (EBNF)

```ebnf
program         = { character_decl | scene_decl } ;

character_decl  = "character" identifier "(" property_list ")" ;
property_list   = [ property { "," property } ] ;
property        = identifier "=" expression ;

scene_decl      = "scene" identifier "{" { statement } "}" ;

statement       = show_stmt
                | hide_stmt
                | say_stmt
                | choice_stmt
                | if_stmt
                | set_stmt
                | goto_stmt
                | wait_stmt
                | transition_stmt
                | play_stmt
                | stop_stmt
                | block_stmt ;

show_stmt       = "show" ("background" string | identifier ["at" position] ["with" string]) ;
hide_stmt       = "hide" (identifier | "background") ;
say_stmt        = "say" identifier string ;
choice_stmt     = "choice" "{" { choice_option } "}" ;
choice_option   = string ["if" expression] "->" (goto_stmt | block_stmt) ;
if_stmt         = "if" expression block_stmt { "else" "if" expression block_stmt } [ "else" block_stmt ] ;
set_stmt        = "set" ["flag"] identifier "=" expression ;
goto_stmt       = "goto" identifier ;
wait_stmt       = "wait" number ;
transition_stmt = "transition" identifier number ;
play_stmt       = "play" ("music" | "sound" | "voice") string [ play_options ] ;
play_options    = { identifier "=" expression } ;
stop_stmt       = "stop" ("music" | "sound" | "voice") [ "fade" "=" number ] ;
block_stmt      = "{" { statement } "}" ;

position        = "left" | "center" | "right" | "(" number "," number ")" ;

expression      = or_expr ;
or_expr         = and_expr { "||" and_expr } ;
and_expr        = equality_expr { "&&" equality_expr } ;
equality_expr   = comparison_expr { ("==" | "!=") comparison_expr } ;
comparison_expr = additive_expr { ("<" | "<=" | ">" | ">=") additive_expr } ;
additive_expr   = multiplicative_expr { ("+" | "-") multiplicative_expr } ;
multiplicative_expr = unary_expr { ("*" | "/" | "%") unary_expr } ;
unary_expr      = ["!" | "-"] primary_expr ;
primary_expr    = number | string | "true" | "false" | identifier | "flag" identifier | "(" expression ")" ;

identifier      = letter { letter | digit | "_" } ;
number          = digit { digit } [ "." digit { digit } ] ;
string          = '"' { character } '"' ;
```

---

## 12. Semantic Rules

### Character Rules

1. Characters must be declared before use in `say` or `show` statements
2. Character IDs must be unique
3. Character names can contain any UTF-8 characters

### Scene Rules

1. Scene IDs must be unique
2. At least one scene must be defined
3. All `goto` targets must reference existing scenes
4. Empty scenes generate a warning

### Variable Rules

1. Variables are dynamically created on first assignment
2. Variable names follow identifier rules
3. Variables have global scope within a script

### Flag Rules

1. Flags are boolean-only variables
2. Flags are accessed with `flag` prefix in conditions
3. Flags default to `false` if not set

### Choice Rules

1. Choices must have at least one option
2. Each option must have a destination (`goto` or block)
3. Conditional options use `if` syntax

---

## 13. Compatibility Policy

### Versioning

NM Script uses semantic versioning:
- **Major** (1.x.x): Breaking changes
- **Minor** (x.1.x): New features, backward compatible
- **Patch** (x.x.1): Bug fixes

### Forward Compatibility

Scripts written for NM Script 1.0 will work with:
- All 1.x versions
- Higher versions may require migration tools

### Deprecation Policy

1. Features are marked deprecated in minor versions
2. Deprecated features generate compiler warnings
3. Deprecated features are removed in next major version

### Migration Path

When breaking changes occur:
1. Migration guide provided in release notes
2. Automated migration tool available
3. Grace period of at least one major version

---

## Appendix A: Complete Example

```nms
// Character definitions
character Hero(name="Alex", color="#00AAFF")
character Sage(name="Elder Sage", color="#FFD700")
character Narrator(name="", color="#AAAAAA")

// Opening scene
scene intro {
    transition fade 1.0
    show background "bg_forest_dawn"

    wait 1.0

    say Narrator "The morning sun broke through the ancient trees..."

    show Hero at center

    say Hero "Today is the day. I must find the Elder Sage."

    play music "exploration_theme"

    choice {
        "Head north" -> goto forest_path
        "Check inventory" -> {
            say Hero "Let me see what I have..."
            set flag checked_inventory = true
            goto intro
        }
        "Wait for a sign" -> goto wait_scene
    }
}

scene forest_path {
    show background "bg_forest_deep"
    transition dissolve 0.5

    say Narrator "The path grew darker as Alex ventured deeper."

    if flag checked_inventory {
        say Hero "Good thing I checked my supplies."
        set preparation_bonus = 10
    }

    move Hero to left duration=1.0

    show Sage at right with "mysterious"

    say Sage "I have been expecting you, young one."

    set points = points + 50

    goto sage_dialogue
}

scene wait_scene {
    wait 2.0

    say Narrator "Nothing happened. Perhaps action is needed."

    goto intro
}

scene sage_dialogue {
    say Sage "The path ahead is treacherous."
    say Sage "But I sense great potential in you."

    say Hero "What must I do?"

    say Sage "First, you must prove your worth."

    choice {
        "I am ready." -> {
            set flag accepted_quest = true
            set points = points + 25
            goto trial_begin
        }
        "I need more time." -> {
            say Sage "Time... is a luxury we may not have."
            goto sage_dialogue
        }
    }
}

scene trial_begin {
    play music "epic_theme" loop=true
    transition fade 0.3

    say Narrator "And so, the trial began..."

    // Trial content would continue here
}
```

---

## Appendix B: Error Codes

| Code | Severity | Description |
|------|----------|-------------|
| E3001 | Error | Undefined character |
| E3002 | Error | Duplicate character definition |
| E3003 | Warning | Unused character |
| E3101 | Error | Undefined scene |
| E3102 | Error | Duplicate scene definition |
| E3103 | Warning | Unused scene |
| E3104 | Warning | Empty scene |
| E3105 | Warning | Unreachable scene |
| E3201 | Error | Undefined variable |
| E3202 | Warning | Unused variable |
| E3301 | Warning | Dead code detected |
| E3601 | Error | Empty choice block |

---

*End of NM Script 1.0 Specification*
