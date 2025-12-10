# NovelMind Versioning and Compatibility Policy

This document defines the versioning scheme, API stability guarantees, and compatibility policies for the NovelMind engine and editor.

## Version Numbering

NovelMind follows [Semantic Versioning 2.0.0](https://semver.org/) with the format `MAJOR.MINOR.PATCH`:

- **MAJOR**: Breaking changes to public APIs
- **MINOR**: New features, backward-compatible
- **PATCH**: Bug fixes, backward-compatible

### Current Versions

| Component | Version | Status |
|-----------|---------|--------|
| NovelMind Engine | 0.1.0 | Alpha |
| NovelMind Editor | 0.1.0 | Alpha |
| NM Script Language | 1.0 | Stable |
| Pack File Format | 1 | Stable |
| Project Format | 1 | Stable |

## Stability Tiers

APIs and file formats are classified into stability tiers:

### Tier 1: Stable

These APIs are considered stable and will maintain backward compatibility:

- **NM Script 1.0 Language Syntax**
  - All documented keywords and constructs
  - Character/scene declarations
  - Dialogue, choice, and control flow statements
  - Built-in functions and operators

- **Pack File Format v1**
  - Header structure
  - Resource table format
  - Encryption scheme (AES-256-GCM)
  - Compression (zlib)

- **Project File Format v1**
  - `project.json` schema
  - Directory structure conventions
  - Asset reference format

- **Save File Format v1**
  - Save slot structure
  - State serialization format
  - Integrity verification

### Tier 2: Provisional

These APIs are functional but may change in minor versions:

- **SceneGraph API**
  - `NovelMind::scene::SceneGraph`
  - `NovelMind::scene::SceneObject`
  - `NovelMind::scene::SceneInspectorAPI`

- **ScriptRuntime API**
  - `NovelMind::scripting::ScriptRuntime`
  - `NovelMind::scripting::VM`
  - `NovelMind::scripting::Value`

- **Audio System 2.0 API**
  - `NovelMind::audio::AudioManager`
  - Channel configuration
  - Voice ducking parameters

- **Localization API**
  - `NovelMind::localization::LocalizationManager`
  - Plural forms handling
  - Import/export formats

- **UI Framework API**
  - `NovelMind::ui::Widget` hierarchy
  - Layout system
  - Event routing

### Tier 3: Experimental

These APIs are under active development and may change significantly:

- **Editor APIs**
  - `NovelMind::editor::EditorRuntimeHost`
  - `NovelMind::editor::VoiceManager`
  - Panel and settings APIs

- **Visual Graph/IR**
  - `NovelMind::scripting::VisualGraph`
  - `NovelMind::scripting::IRNode`
  - Round-trip conversion

- **Plugin System** (planned)
  - Custom node registration
  - Asset importer extensions

## Breaking Change Policy

### What Constitutes a Breaking Change

1. **API Changes**
   - Removing or renaming public classes, methods, or functions
   - Changing method signatures (parameters, return types)
   - Changing behavior of documented APIs
   - Removing enum values

2. **File Format Changes**
   - Changing file structure requiring migration
   - Removing support for older format versions
   - Changing encryption or compression schemes

3. **Language Changes**
   - Removing NM Script keywords or constructs
   - Changing semantic meaning of statements
   - Making previously valid scripts invalid

### How Breaking Changes Are Introduced

1. **Announcement**: Breaking changes are documented at least one minor version before implementation

2. **Deprecation**: Affected APIs are marked deprecated with warnings:
   ```cpp
   [[deprecated("Use newMethod() instead. Will be removed in v1.0.0")]]
   void oldMethod();
   ```

3. **Migration Path**: Clear documentation and tools for migration are provided

4. **Version Bump**: Breaking changes result in a MAJOR version increment (after 1.0.0)

## Format Migration

### Pack File Format

When the pack format changes:

1. New format gets a new version number
2. Runtime can read all older format versions
3. Editor exports to latest format only
4. Migration tool provided for batch conversion

### Project Format

Project format changes:

1. Editor detects project format version on open
2. Offers automatic migration if needed
3. Creates backup before migration
4. Documents all changes in release notes

### Save Format

Save format compatibility:

1. New game versions can always load older saves
2. Older game versions cannot load newer saves (graceful error)
3. Save version stored in save file header

## Deprecated APIs

Currently deprecated APIs (will be removed in specified version):

| API | Deprecated In | Remove In | Replacement |
|-----|---------------|-----------|-------------|
| *None currently* | - | - | - |

## NM Script Language Versioning

### Language Version Declaration

Projects can declare their NM Script version:

```json
{
    "nmScriptVersion": "1.0"
}
```

### Language Evolution

1. **1.x**: Backward compatible additions
   - New optional keywords
   - New built-in functions
   - Extended syntax (backward compatible)

2. **2.0**: May include breaking changes
   - Reserved keywords becoming active
   - Stricter parsing rules
   - Semantic changes

### Reserved Keywords

The following keywords are reserved for future use:

```
async await yield import export module
class interface struct trait impl
match switch case default
try catch throw finally
public private protected
static const let var
```

## Compatibility Testing

### Automated Compatibility Tests

- Unit tests for all Tier 1 and Tier 2 APIs
- Integration tests with sample projects
- Regression tests for file format loading
- Cross-version save/load tests

### Compatibility Matrix

Maintained in CI:

| Engine Version | Script v1.0 | Pack v1 | Project v1 | Save v1 |
|---------------|-------------|---------|------------|---------|
| 0.1.x         | ✓           | ✓       | ✓          | ✓       |

## Support Lifecycle

### Version Support

- **Current**: Full support, bug fixes, security updates
- **Previous Minor**: Security updates only for 6 months
- **Previous Major**: Security updates only for 12 months

### End of Life

When a version reaches end of life:

1. Announcement 3 months in advance
2. Final security-only release
3. Archived but available for download
4. Migration documentation maintained

## Reporting Compatibility Issues

If you encounter a compatibility issue:

1. Check the [Known Issues](./KNOWN_ISSUES.md) document
2. Search existing [GitHub Issues](https://github.com/VisageDvachevsky/NM-/issues)
3. Open a new issue with:
   - NovelMind version numbers
   - Steps to reproduce
   - Expected vs actual behavior
   - Sample project if applicable

## Changelog

All changes are documented in [CHANGELOG.md](../CHANGELOG.md) with:

- Version number and date
- Breaking changes (highlighted)
- New features
- Bug fixes
- Deprecations
