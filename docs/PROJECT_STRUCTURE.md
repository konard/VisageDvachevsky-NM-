# NovelMind Project Structure

This document describes the standard directory structure for NovelMind visual novel projects.

## Standard Project Layout

```
MyProject/
├── project.json           # Project configuration and metadata
├── .gitignore             # Version control exclusions
│
├── assets/                # All project assets
│   ├── backgrounds/       # Background images
│   │   ├── day/
│   │   └── night/
│   ├── characters/        # Character sprites
│   │   ├── hero/
│   │   │   ├── expressions/
│   │   │   └── poses/
│   │   └── sidekick/
│   ├── ui/                # UI elements
│   │   ├── buttons/
│   │   ├── frames/
│   │   └── icons/
│   ├── effects/           # Visual effects
│   ├── fonts/             # Custom fonts
│   ├── voice/             # Voice-over files
│   │   ├── hero/
│   │   └── narrator/
│   ├── music/             # Background music
│   └── sounds/            # Sound effects
│
├── scripts/               # NM Script source files
│   ├── main.nms           # Entry point script
│   ├── characters.nms     # Character definitions
│   ├── scenes/            # Scene scripts
│   │   ├── prologue.nms
│   │   ├── chapter1/
│   │   └── endings/
│   └── common/            # Shared scripts
│
├── locales/               # Localization files
│   ├── en.json            # English strings
│   ├── ja.json            # Japanese strings
│   └── templates/         # Translation templates
│
├── data/                  # Game data files
│   ├── config.json        # Runtime configuration
│   └── achievements.json  # Achievement definitions
│
├── docs/                  # Project documentation
│   └── design/            # Design documents
│
└── Build/                 # Build outputs (ignored by VCS)
    ├── debug/
    └── release/
```

## Directory Descriptions

### `/project.json`

The project configuration file containing:

```json
{
    "name": "My Visual Novel",
    "version": "1.0.0",
    "author": "Author Name",
    "description": "A compelling visual novel...",
    "startScene": "prologue",
    "defaultResolution": {
        "width": 1920,
        "height": 1080
    },
    "locales": ["en", "ja"],
    "defaultLocale": "en",
    "nmScriptVersion": "1.0"
}
```

### `/assets/`

Contains all media assets used by the project.

**Organization Guidelines:**
- Use subdirectories to group related assets
- Use descriptive, lowercase names with underscores
- Keep character assets in character-named folders
- Voice files should match dialogue line naming conventions

**Supported Formats:**
- Images: PNG, JPEG, WebP
- Audio: OGG, WAV, MP3, FLAC
- Fonts: TTF, OTF

### `/scripts/`

Contains NM Script source files.

**Organization Guidelines:**
- `main.nms` is the entry point
- Group scene scripts by chapter or story section
- Keep shared/utility scripts in `common/`
- Character definitions can be centralized or per-chapter

### `/locales/`

Contains localization files for multi-language support.

**Organization Guidelines:**
- One file per locale (e.g., `en.json`, `ja.json`)
- Use ISO 639-1 language codes
- Keep template files for translators

**File Format:**
```json
{
    "greeting": "Hello!",
    "farewell": "Goodbye!",
    "item_count_one": "{count} item",
    "item_count_other": "{count} items"
}
```

### `/data/`

Contains game configuration and data files.

- `config.json`: Runtime settings
- Achievement definitions
- Custom game data

### `/Build/`

Build output directory. **Always ignored by version control.**

Contains:
- Compiled scripts
- Packed assets
- Final executables

## Asset Naming Conventions

### Backgrounds

```
location_variant_time.png

Examples:
school_classroom_day.png
school_classroom_night.png
park_fountain.png
```

### Character Sprites

```
character/expression_pose.png

Examples:
hero/happy_standing.png
hero/sad_sitting.png
sidekick/neutral_default.png
```

### Voice Files

```
character_lineId.ogg

Examples:
hero_001_greeting.ogg
narrator_intro_001.ogg
sidekick_ch1_045.ogg
```

Pattern recognition supports:
- `{character}_{id}.{ext}`
- `{character}_{number}_{description}.{ext}`
- `{scene}_{character}_{line}.{ext}`

### Music

```
type_name.ogg

Examples:
bgm_main_theme.ogg
bgm_battle.ogg
bgm_sad_moment.ogg
```

### Sound Effects

```
category_name.ogg

Examples:
ui_click.ogg
sfx_door_open.ogg
ambient_rain.ogg
```

## Version Control Best Practices

### Files to Include

- All source scripts (`.nms`)
- Localization files (`.json`, `.csv`, `.po`)
- Project configuration (`project.json`)
- Documentation (`/docs/`)
- Asset source files (images, audio)

### Files to Exclude

Use the provided `.gitignore` template to exclude:

- Build outputs (`/Build/`)
- Editor cache (`.nmcache/`)
- Thumbnails and previews
- Log files
- Local configuration
- OS-specific files
- IDE configuration

## Multi-Developer Workflow

### Branch Strategy

```
main              # Stable releases
├── develop       # Integration branch
│   ├── feature/* # Feature branches
│   └── fix/*     # Bug fix branches
└── release/*     # Release preparation
```

### Asset Handling

For large binary assets:
- Consider Git LFS for voice files and music
- Use asset bundles for team distribution
- Document asset ownership/responsibility

### Merge Conflicts

Common conflict sources:
- `project.json` version changes
- Localization string additions
- Script modifications

Resolution:
- Prefer JSON merge tools for data files
- Use NM Script's module system to reduce conflicts
- Communicate about shared resources

## Creating New Projects

### From Template

1. Open NovelMind Editor
2. File > New Project
3. Select template:
   - Empty Project
   - Kinetic Novel
   - Branching Story
4. Choose location
5. Configure project settings

### Manual Setup

1. Create project directory
2. Copy `project.gitignore` as `.gitignore`
3. Create `project.json` with required fields
4. Create standard directories:
   - `assets/`
   - `scripts/`
   - `locales/`
5. Add `scripts/main.nms` entry point

## Project Migration

When migrating from older versions:

1. Back up the entire project
2. Open in new editor version
3. Follow migration prompts
4. Verify all assets load correctly
5. Test gameplay from start to finish
6. Update `project.json` version if needed
