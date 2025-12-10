# NovelMind Troubleshooting Guide

This guide helps you diagnose and resolve common issues when using NovelMind.

## Table of Contents

- [Editor Issues](#editor-issues)
- [Build Issues](#build-issues)
- [Runtime Issues](#runtime-issues)
- [Script Issues](#script-issues)
- [Asset Issues](#asset-issues)
- [Performance Issues](#performance-issues)
- [Platform-Specific Issues](#platform-specific-issues)

---

## Editor Issues

### Editor Won't Start

**Symptoms**: The editor crashes on launch or shows a blank window.

**Solutions**:

1. **Check System Requirements**
   - C++20-compatible system
   - OpenGL 3.3 or higher
   - SDL2 installed

2. **Reset Editor Settings**
   ```bash
   # Delete settings file to reset
   rm ~/.novelmind/editor_settings.json
   ```

3. **Check Graphics Drivers**
   - Update to latest GPU drivers
   - Try running with software rendering:
   ```bash
   SDL_RENDER_DRIVER=software ./novelmind_editor
   ```

4. **Check Console Output**
   - Run from terminal to see error messages:
   ```bash
   ./novelmind_editor 2>&1 | tee editor.log
   ```

### Editor Freezes or Hangs

**Symptoms**: The editor becomes unresponsive.

**Solutions**:

1. **Check for Infinite Loops in Script**
   - Open the Console panel for error messages
   - Check for recursive scene transitions (scene A -> scene B -> scene A)

2. **Large Asset Loading**
   - Wait for assets to finish loading (check progress bar)
   - Consider optimizing large images

3. **Force Close and Recover**
   - NovelMind auto-saves periodically
   - Check `ProjectSettings/autosave/` for recovery files

### Changes Not Saving

**Symptoms**: Edits disappear after restarting the editor.

**Solutions**:

1. **Check Write Permissions**
   - Ensure the project folder is writable
   - Don't work in system directories

2. **Verify Project Path**
   - Avoid special characters in project path
   - Keep path length reasonable (< 200 characters)

3. **Manual Save**
   - Use `Ctrl+S` to force save
   - Check **File > Recent Projects** for correct path

---

## Build Issues

### Build Fails to Start

**Symptoms**: Clicking Build does nothing or shows an error.

**Solutions**:

1. **Check Build Configuration**
   - Open **Build > Build Settings**
   - Verify target platform is selected
   - Ensure output path exists and is writable

2. **Validate Project**
   - Run **Tools > Validate Project** first
   - Fix all reported errors before building

3. **Check Disk Space**
   - Build process needs temporary space
   - Ensure at least 500MB free space

### Missing Resources in Build

**Symptoms**: Built game is missing images, audio, or other assets.

**Solutions**:

1. **Check Asset References**
   - Verify all asset paths in scripts are correct
   - Use relative paths, not absolute paths

2. **Rebuild Asset Database**
   - **Assets > Rebuild Database**
   - Check for errors in Console

3. **Check Pack Settings**
   - Ensure assets are included in pack configuration
   - Review **Build > Pack Settings**

### Build Crashes on Specific Platform

**Symptoms**: Build works on one platform but crashes on another.

**Solutions**:

1. **Check Platform Requirements**
   - Some features require specific OS versions
   - Check minimum requirements for target platform

2. **Test Asset Compatibility**
   - Verify image formats are cross-platform (PNG, JPEG)
   - Check audio codecs (OGG recommended)

3. **Review Console Logs**
   - Check `Build/logs/` for detailed error messages

---

## Runtime Issues

### Game Won't Launch

**Symptoms**: The built game executable doesn't start.

**Solutions**:

1. **Check Dependencies**
   - Windows: Ensure Visual C++ Runtime is installed
   - Linux: Check for missing libraries with `ldd ./game`
   - macOS: Grant security permissions in System Preferences

2. **Run from Terminal**
   ```bash
   ./my_game 2>&1 | tee game.log
   ```

3. **Check Pack File Integrity**
   - Ensure `.pack` files weren't corrupted during transfer
   - Re-build if necessary

### Save/Load Not Working

**Symptoms**: Game state doesn't persist between sessions.

**Solutions**:

1. **Check Write Permissions**
   - Save directory must be writable
   - Default: `~/.novelmind/saves/` (Linux/macOS) or `%APPDATA%\NovelMind\saves\` (Windows)

2. **Verify Save Data**
   - Check if save files are created
   - Look for corruption messages in console

3. **Test in Debug Mode**
   - Build with debug enabled
   - Check save/load callbacks in console

### Audio Not Playing

**Symptoms**: No sound or music in the game.

**Solutions**:

1. **Check Audio Files**
   - Verify files exist in Assets/Music and Assets/Sounds
   - Supported formats: OGG, WAV, MP3

2. **Check Audio Settings**
   - Verify system volume isn't muted
   - Check in-game volume settings

3. **Audio Driver Issues**
   - Linux: Ensure PulseAudio/ALSA is running
   - Try different audio backend:
   ```bash
   SDL_AUDIODRIVER=alsa ./my_game
   ```

---

## Script Issues

### Script Syntax Errors

**Symptoms**: Red errors in the Console about script parsing.

**Solutions**:

1. **Check Error Location**
   - Error message includes line and column
   - Navigate to the location in Script Editor

2. **Common Syntax Issues**
   ```nm
   # Wrong: Missing quotes
   say hero Hello world

   # Correct
   say hero "Hello world"

   # Wrong: Unclosed brace
   scene test {
       say hero "Hello"

   # Correct
   scene test {
       say hero "Hello"
   }
   ```

3. **Use Script Validator**
   - **Tools > Validate Script**
   - Shows all errors and warnings

### Variables Not Updating

**Symptoms**: Variable values don't change as expected.

**Solutions**:

1. **Check Variable Scope**
   - Global variables persist across scenes
   - Local variables reset on scene change

2. **Verify Condition Syntax**
   ```nm
   # Wrong: Assignment instead of comparison
   if score = 10 { ... }

   # Correct: Comparison
   if score == 10 { ... }
   ```

3. **Debug with Console**
   - Add debug output:
   ```nm
   debug "Current score: " + score
   ```

### Choices Not Appearing

**Symptoms**: Choice menu doesn't show or has wrong options.

**Solutions**:

1. **Check Choice Conditions**
   ```nm
   choice {
       "Option A" if flag_unlocked -> scene_a
       "Option B" -> scene_b
   }
   ```
   - Verify conditions evaluate to true

2. **Validate Choice Structure**
   - Each option needs text and target
   - Targets must exist

3. **Check UI Settings**
   - Choice menu might be hidden
   - Verify UI layer is visible

---

## Asset Issues

### Images Not Displaying

**Symptoms**: Characters or backgrounds show as blank or placeholder.

**Solutions**:

1. **Check File Path**
   - Use forward slashes: `Assets/Characters/hero.png`
   - Paths are case-sensitive on Linux

2. **Verify Image Format**
   - Supported: PNG, JPEG, BMP, TGA
   - Recommended: PNG for transparency

3. **Check Image Dimensions**
   - Maximum texture size depends on GPU
   - Typical limit: 4096x4096 pixels

4. **Rebuild Asset Database**
   - **Assets > Rebuild Database**

### Asset Import Failing

**Symptoms**: Can't add new assets to the project.

**Solutions**:

1. **Check File Format**
   - Images: PNG, JPEG, BMP, TGA
   - Audio: OGG, WAV, MP3
   - Fonts: TTF, OTF

2. **Check File Size**
   - Very large files may fail to import
   - Consider compressing before import

3. **Check File Names**
   - Avoid special characters
   - Use only alphanumeric, underscore, hyphen

### Hot-Reload Not Working

**Symptoms**: Asset changes aren't reflected in editor.

**Solutions**:

1. **Check Hot-Reload Settings**
   - **Edit > Preferences > Hot Reload**: Enabled
   - Set appropriate interval

2. **Manual Refresh**
   - Right-click asset > **Reimport**
   - Or **Assets > Refresh All**

3. **File Watcher Issues**
   - Linux: Check inotify limits
   ```bash
   sudo sysctl fs.inotify.max_user_watches=524288
   ```

---

## Performance Issues

### Slow Editor Performance

**Symptoms**: Editor feels sluggish or laggy.

**Solutions**:

1. **Reduce Asset Preview Quality**
   - **Edit > Preferences > Preview Quality**: Low

2. **Close Unused Panels**
   - Each panel consumes resources

3. **Optimize Large Projects**
   - Archive unused assets
   - Split into multiple pack files

4. **Check System Resources**
   - Close other applications
   - Monitor CPU/RAM usage

### Slow Game Performance

**Symptoms**: Built game runs slowly.

**Solutions**:

1. **Profile the Game**
   - Enable profiling in debug build
   - Check `profile_results.json`

2. **Optimize Assets**
   - Reduce image resolutions
   - Compress audio files
   - Use texture atlases

3. **Reduce Effects**
   - Simplify transitions
   - Limit simultaneous animations

4. **Check Update Loop**
   - Avoid complex logic in scene update
   - Cache frequently used values

### High Memory Usage

**Symptoms**: Game uses too much RAM.

**Solutions**:

1. **Optimize Image Sizes**
   - Don't use larger images than needed
   - Scale down backgrounds to display resolution

2. **Unload Unused Assets**
   - Assets are cached by default
   - Clear cache between chapters:
   ```nm
   clear_cache
   ```

3. **Check for Memory Leaks**
   - Build with ASAN enabled:
   ```bash
   cmake -DNOVELMIND_ENABLE_ASAN=ON ..
   ```

---

## Platform-Specific Issues

### Windows

**Issue**: "VCRUNTIME140.dll not found"
- Install Visual C++ Redistributable 2022

**Issue**: Windows Defender blocks executable
- Add game folder to exclusions
- Or sign the executable for distribution

**Issue**: Path too long error
- Keep project paths short
- Enable long path support in Windows

### Linux

**Issue**: "libSDL2 not found"
```bash
sudo apt-get install libsdl2-2.0-0  # Debian/Ubuntu
sudo dnf install SDL2               # Fedora
```

**Issue**: No permission to execute
```bash
chmod +x ./my_game
```

**Issue**: Wayland display issues
```bash
SDL_VIDEODRIVER=x11 ./my_game
```

### macOS

**Issue**: "Cannot be opened because the developer cannot be verified"
- System Preferences > Security & Privacy > Allow
- Or right-click > Open

**Issue**: Retina display scaling
- Check HiDPI settings in game configuration
- Ensure assets have @2x variants if needed

---

## Getting More Help

If you can't resolve your issue:

1. **Search Existing Issues**
   - [GitHub Issues](https://github.com/VisageDvachevsky/NM-/issues)

2. **Report a Bug**
   - Include NovelMind version
   - Include operating system
   - Include steps to reproduce
   - Attach relevant log files

3. **Check Documentation**
   - [Architecture Overview](architecture_overview.md)
   - [NM Script Specification](nm_script_specification.md)

4. **Enable Verbose Logging**
   ```bash
   NOVELMIND_LOG_LEVEL=debug ./novelmind_editor
   ```

---

## Log File Locations

| Platform | Location |
|----------|----------|
| Windows | `%APPDATA%\NovelMind\logs\` |
| Linux | `~/.novelmind/logs/` |
| macOS | `~/Library/Application Support/NovelMind/logs/` |

Include these logs when reporting issues.
