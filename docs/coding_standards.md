# Coding Standards

This document defines the coding standards and conventions for the NovelMind project.

## Language Standard

- **C++ Standard**: C++20 (with C++23 features where beneficial)
- **Compiler Support**: GCC 11+, Clang 14+, MSVC 2022+

## Naming Conventions

### General Rules

| Element | Style | Example |
|---------|-------|---------|
| Classes/Structs | PascalCase | `SceneManager`, `DialogueNode` |
| Functions/Methods | camelCase | `loadTexture()`, `getCurrentScene()` |
| Variables | camelCase | `playerName`, `currentFrame` |
| Member Variables | m_ prefix + camelCase | `m_windowHandle`, `m_isRunning` |
| Constants | ALL_CAPS | `MAX_SPRITES`, `DEFAULT_WIDTH` |
| Enums | PascalCase | `enum class RenderMode` |
| Enum Values | PascalCase | `RenderMode::Immediate` |
| Namespaces | PascalCase | `NovelMind::Core`, `NovelMind::VFS` |
| Macros | ALL_CAPS with NOVELMIND_ prefix | `NOVELMIND_ASSERT`, `NOVELMIND_DEBUG` |
| Template Parameters | PascalCase | `template<typename ValueType>` |
| File Names | snake_case | `scene_manager.hpp`, `vfs_interface.cpp` |

### Namespace Structure

```cpp
namespace NovelMind {
    namespace Core { }      // Core utilities, types, result
    namespace Platform { }  // Platform abstraction
    namespace VFS { }       // Virtual file system
    namespace Renderer { }  // 2D rendering
    namespace Scripting { } // Script engine
    namespace Scene { }     // Scene management
    namespace Audio { }     // Audio system
    namespace Input { }     // Input handling
    namespace Save { }      // Save/load system
}
```

**Note**: The top-level namespace is `NovelMind`, not `nm`. This provides:
- Clear, self-documenting namespace that is recognizable in any codebase
- No conflicts with other libraries
- Professional, polished appearance

## Code Formatting

### Indentation and Spacing

- **Indentation**: 4 spaces (no tabs)
- **Line Length**: Maximum 100 characters
- **Braces**: Allman style (opening brace on new line)

```cpp
namespace NovelMind::Core
{

class Application
{
public:
    explicit Application(const Config& config);

    void run();
    void shutdown();

private:
    void processEvents();
    void update(float deltaTime);
    void render();

    bool m_isRunning;
    Config m_config;
};

} // namespace NovelMind::Core
```

### Function Definitions

```cpp
ReturnType ClassName::functionName(ParamType1 param1, ParamType2 param2)
{
    if (condition)
    {
        doSomething();
    }
    else
    {
        doSomethingElse();
    }

    return result;
}
```

### Control Structures

```cpp
// If statements
if (condition)
{
    action();
}

// Switch statements
switch (value)
{
    case Value::First:
        handleFirst();
        break;

    case Value::Second:
        handleSecond();
        break;

    default:
        handleDefault();
        break;
}

// Loops
for (size_t i = 0; i < count; ++i)
{
    process(items[i]);
}

for (const auto& item : container)
{
    process(item);
}

while (condition)
{
    iterate();
}
```

## Header Files

### Header Guards

Use `#pragma once` for header guards:

```cpp
#pragma once

#include <cstdint>
#include "NovelMind/core/types.hpp"

namespace NovelMind::VFS
{
    // ...
}
```

### Include Order

1. Corresponding header (for .cpp files)
2. C++ standard library headers
3. Third-party library headers
4. Project headers (sorted alphabetically)

```cpp
#include "scene_manager.hpp"

#include <algorithm>
#include <memory>
#include <vector>

#include <SDL.h>

#include "NovelMind/core/logger.hpp"
#include "NovelMind/renderer/sprite.hpp"
#include "NovelMind/vfs/resource.hpp"
```

### Forward Declarations

Prefer forward declarations in headers when possible:

```cpp
// In header
namespace NovelMind::Renderer
{
    class Texture;
    class Sprite;
}

class SceneManager
{
    std::unique_ptr<NovelMind::Renderer::Sprite> m_background;
};
```

## Classes and Structs

### Class Layout

```cpp
class ClassName
{
public:
    // Types and aliases
    using Ptr = std::unique_ptr<ClassName>;

    // Static constants
    static constexpr int MAX_COUNT = 100;

    // Constructors and destructor
    ClassName();
    explicit ClassName(int value);
    ~ClassName();

    // Copy/move operations
    ClassName(const ClassName&) = delete;
    ClassName& operator=(const ClassName&) = delete;
    ClassName(ClassName&&) noexcept;
    ClassName& operator=(ClassName&&) noexcept;

    // Public interface
    void publicMethod();
    [[nodiscard]] int getValue() const;

protected:
    // Protected interface
    void protectedMethod();

private:
    // Private methods
    void privateHelper();

    // Member variables (always last)
    int m_value;
    std::string m_name;
};
```

### Struct vs Class

- Use `struct` for plain data aggregates (POD-like types)
- Use `class` for types with invariants and behavior

```cpp
// Data aggregate
struct Vec2
{
    float x;
    float y;
};

// Type with behavior
class Transform
{
public:
    void setPosition(const Vec2& pos);
    void setRotation(float angle);

private:
    Vec2 m_position;
    float m_rotation;
    bool m_dirty;
};
```

## Modern C++ Features

### Use Smart Pointers

```cpp
// Unique ownership
std::unique_ptr<Texture> texture = std::make_unique<Texture>(path);

// Shared ownership (use sparingly)
std::shared_ptr<Resource> resource = std::make_shared<Resource>(id);

// Non-owning reference
Texture* rawPtr = texture.get();
```

### Use `auto` Judiciously

```cpp
// Good: obvious from context or long type names
auto it = container.begin();
auto texture = resourceManager.loadTexture("sprite.png");

// Avoid: hides important type information
auto value = calculateValue();  // What type is this?
int value = calculateValue();   // Better
```

### Use `constexpr` and `const`

```cpp
constexpr int BUFFER_SIZE = 1024;
constexpr float PI = 3.14159265f;

const std::string& getName() const { return m_name; }
void process(const std::vector<int>& data);
```

### Use `[[nodiscard]]`

```cpp
[[nodiscard]] bool initialize();
[[nodiscard]] std::optional<Resource> loadResource(const std::string& id);
```

### Use Enum Classes

```cpp
enum class TextAlignment
{
    Left,
    Center,
    Right
};

// Usage
TextAlignment align = TextAlignment::Center;
```

## Error Handling

### No Exceptions

Exceptions are disabled in the project. Use explicit error handling:

```cpp
// Result type for operations that can fail
template<typename T, typename E = std::string>
class Result
{
public:
    static Result ok(T value);
    static Result error(E error);

    [[nodiscard]] bool isOk() const;
    [[nodiscard]] bool isError() const;
    [[nodiscard]] T& value();
    [[nodiscard]] E& error();
};

// Usage
Result<Texture> result = loadTexture("image.png");
if (result.isOk())
{
    useTexture(result.value());
}
else
{
    log::error("Failed to load texture: {}", result.error());
}
```

### Assertions

Use assertions for programmer errors:

```cpp
#define NOVELMIND_ASSERT(condition, message) \
    do { if (!(condition)) { NovelMind::Core::assertFailed(#condition, message, __FILE__, __LINE__); } } while(0)

void processItem(Item* item)
{
    NOVELMIND_ASSERT(item != nullptr, "Item must not be null");
    // ...
}
```

## Documentation

### File Headers

Every source file should have a header comment:

```cpp
/**
 * @file scene_manager.hpp
 * @brief Scene management and lifecycle handling
 *
 * This module provides the SceneManager class which handles
 * scene loading, transitions, and object management.
 */
```

### Class Documentation

```cpp
/**
 * @brief Manages scene lifecycle and transitions
 *
 * The SceneManager is responsible for loading scenes, managing
 * scene objects, and handling transitions between scenes.
 */
class SceneManager
{
public:
    /**
     * @brief Load a scene by identifier
     * @param sceneId Unique scene identifier
     * @return Result indicating success or error
     */
    [[nodiscard]] Result<void> loadScene(const std::string& sceneId);
};
```

### Inline Comments

- Write comments for non-obvious logic
- Avoid commenting obvious code
- Keep comments up-to-date with code changes

```cpp
// Good: explains why
// Use power-of-two size for texture atlas to ensure GPU compatibility
constexpr int ATLAS_SIZE = 2048;

// Bad: states the obvious
// Increment counter
++counter;
```

## Testing

### Test File Naming

- Unit tests: `test_<module_name>.cpp`
- Integration tests: `integration_<feature>.cpp`

### Test Structure

```cpp
#include <catch2/catch_test_macros.hpp>
#include "NovelMind/vfs/virtual_fs.hpp"

TEST_CASE("VirtualFS loads resources correctly", "[vfs]")
{
    NovelMind::VFS::VirtualFS vfs;

    SECTION("can load existing resource")
    {
        auto result = vfs.readFile("test_resource");
        REQUIRE(result.isOk());
    }

    SECTION("returns error for missing resource")
    {
        auto result = vfs.readFile("nonexistent");
        REQUIRE(result.isError());
    }
}
```

## Build Configuration

### CMake Standards

- Minimum CMake version: 3.20
- Use modern CMake targets
- Avoid global settings

```cmake
target_compile_features(engine_core PUBLIC cxx_std_20)
target_compile_options(engine_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
```

### Warning Levels

- Enable maximum warnings in development
- Treat warnings as errors in CI
- Document any suppressed warnings

## Code Review Checklist

Before submitting code for review:

- [ ] Code follows naming conventions
- [ ] No magic numbers (use named constants)
- [ ] Public API is documented
- [ ] Error cases are handled
- [ ] No memory leaks (RAII used)
- [ ] Tests written for new functionality
- [ ] No compiler warnings
- [ ] Code compiles on all target platforms
