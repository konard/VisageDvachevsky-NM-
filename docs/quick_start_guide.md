# NovelMind Quick Start Guide

This guide will help you create your first visual novel using NovelMind in just a few minutes.

## Prerequisites

Before you begin, ensure you have:

- NovelMind Editor installed (download from Releases or build from source)
- Basic understanding of visual novel concepts (characters, scenes, dialogue, choices)

## Step 1: Create a New Project

1. Launch the NovelMind Editor
2. Click **File > New Project** or press `Ctrl+N`
3. Enter your project name (e.g., "MyFirstNovel")
4. Choose a project location
5. Select a template:
   - **Visual Novel** - Standard branching narrative with choices
   - **Kinetic Novel** - Linear story without player choices
   - **Dating Sim** - Character-focused with affection tracking

Click **Create** to generate your project.

## Step 2: Understand the Editor Layout

The editor has several panels:

| Panel | Purpose |
|-------|---------|
| **Scene View** | Visual preview of your current scene |
| **StoryGraph** | Node-based narrative flow editor |
| **Assets** | Manage images, audio, and other resources |
| **Inspector** | Edit properties of selected objects |
| **Timeline** | Animation and timing control |
| **Console** | View logs and debug messages |

You can drag panels to rearrange them or use **Window** menu to show/hide panels.

## Step 3: Add Your First Character

### Using the Editor

1. Open the **Assets** panel
2. Right-click in the **Characters** folder
3. Select **Create > Character**
4. Fill in the character details:
   - **ID**: `hero` (internal identifier)
   - **Display Name**: `Alex` (shown to players)
   - **Color**: `#4A90D9` (for dialogue name)
5. Add character sprites by dragging images into the character folder

### Using NM Script

Alternatively, create characters in code:

```nm
character hero(name="Alex", color="#4A90D9")
character friend(name="Sam", color="#D94A4A")
```

## Step 4: Create Your First Scene

### Using StoryGraph

1. Open the **StoryGraph** panel
2. Right-click on the canvas and select **Add > Scene Node**
3. Name the scene `intro`
4. Double-click to enter the scene

Inside the scene:
1. Add a **Dialogue Node** - right-click > **Add > Dialogue**
2. Set the speaker to `hero`
3. Enter text: "Welcome to my visual novel!"
4. Connect nodes by dragging from output port to input port

### Using NM Script

```nm
scene intro {
    show hero at center
    say hero "Welcome to my visual novel!"
    say hero "This is a demonstration of NovelMind."
}
```

## Step 5: Add a Background

1. Import a background image:
   - Drag an image file into **Assets > Backgrounds**
   - Or use **Assets > Import** button
2. In Scene View or StoryGraph:
   - Add a **Background Node**
   - Select your imported image
   - The background will display in Scene View

```nm
scene intro {
    background "classroom.png"
    show hero at center
    say hero "Here we are in the classroom."
}
```

## Step 6: Add Player Choices

Choices create branching narratives:

### Using StoryGraph

1. Add a **Choice Node** after your dialogue
2. Add choice options:
   - "Talk to the friend"
   - "Explore alone"
3. Connect each option to different scene nodes

### Using NM Script

```nm
scene decision {
    say hero "What should I do?"

    choice {
        "Talk to Sam" -> talk_to_sam
        "Explore alone" -> explore
    }
}

scene talk_to_sam {
    show friend at left
    say friend "Hey! I was hoping you'd come over."
}

scene explore {
    hide hero
    say "You decide to explore the area by yourself..."
}
```

## Step 7: Test Your Novel

### Play-In-Editor

1. Click the **Play** button (or press `F5`)
2. Your novel will run directly in the editor
3. Test all dialogue and choices
4. Click **Stop** to return to editing

### Quick Test Features

- **Jump to Scene**: Right-click a scene node > **Jump Here**
- **Variable Inspector**: View current state values
- **Active Node Highlight**: See which node is executing

## Step 8: Add Audio

### Background Music

1. Import audio files to **Assets > Music**
2. Add a **Play Music Node** in StoryGraph
3. Select your music file

```nm
scene intro {
    music "theme.ogg" fade 2.0
    say hero "Can you hear the music?"
}
```

### Voice Acting

1. Import voice files to **Assets > Voice**
2. Open **Voice Manager** panel
3. Bind voice files to dialogue lines
4. NovelMind can auto-bind by naming convention: `scene_speaker_line.ogg`

```nm
say hero "Hello there!" voice="intro_hero_001.ogg"
```

## Step 9: Add Effects and Transitions

### Scene Transitions

```nm
scene end_of_chapter {
    say hero "This chapter is over..."
    transition fade 1.5
    goto chapter_2
}
```

### Character Animations

```nm
scene dramatic {
    show hero at left
    animate hero move to center duration 1.0 ease easeOutQuad
    say hero "Something important is happening!"
    shake screen intensity 5 duration 0.3
}
```

## Step 10: Build Your Novel

1. Go to **Build > Build Settings**
2. Select target platform(s):
   - Windows
   - Linux
   - macOS
3. Configure build options:
   - **Pack Resources**: Encrypt and compress assets
   - **Include Debug**: For testing builds
4. Click **Build**
5. Find your executable in the `Build/` folder

## Project Structure

After creating your project, you'll have:

```
MyFirstNovel/
├── Assets/
│   ├── Backgrounds/    # Scene backgrounds
│   ├── Characters/     # Character sprites
│   ├── Music/          # Background music
│   ├── Sounds/         # Sound effects
│   ├── Voice/          # Voice acting files
│   └── UI/             # Interface elements
├── Scripts/
│   └── main.nms        # Main script file
├── Scenes/             # Scene data files
├── Localization/       # Translation files
└── ProjectSettings/
    └── project.json    # Project configuration
```

## Next Steps

Now that you've created your first novel, explore these features:

- **[NM Script Specification](nm_script_specification.md)** - Full language reference
- **[Timeline Editor]** - Create cinematic sequences
- **[Localization Pipeline]** - Add multiple languages
- **[Voice Manager]** - Organize voice acting

## Tips for Beginners

1. **Start Simple**: Create a short story first before tackling a large project
2. **Use Templates**: Built-in templates show best practices
3. **Test Often**: Use Play-In-Editor frequently during development
4. **Organize Assets**: Keep folders clean from the start
5. **Save Regularly**: Use `Ctrl+S` and enable auto-save in settings

## Getting Help

- **[Issue Tracker](https://github.com/VisageDvachevsky/NM-/issues)** - Report bugs or request features
- **[Documentation](.)** - Full documentation index
- **Console Panel** - Check for warnings and errors

Happy storytelling with NovelMind!
