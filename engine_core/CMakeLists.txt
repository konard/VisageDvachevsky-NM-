add_library(engine_core STATIC
    # Core
    src/core/logger.cpp
    src/core/application.cpp
    src/core/timer.cpp

    # Platform
    src/core/platform_sdl.cpp

    # VFS
    src/vfs/virtual_fs.cpp
    src/vfs/memory_fs.cpp
    src/vfs/pack_reader.cpp

    # Renderer
    src/renderer/renderer.cpp
    src/renderer/texture.cpp
    src/renderer/sprite.cpp
    src/renderer/font.cpp

    # Scripting
    src/scripting/interpreter.cpp
    src/scripting/vm.cpp

    # Scene
    src/scene/scene_manager.cpp
    src/scene/scene_object.cpp

    # Input
    src/input/input_manager.cpp

    # Audio
    src/audio/audio_manager.cpp

    # Save
    src/save/save_manager.cpp
)

target_include_directories(engine_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(engine_core
    PRIVATE
        nm_compiler_options
)

# Find SDL2 (optional for now, will be required later)
find_package(SDL2 QUIET)
if(SDL2_FOUND)
    target_link_libraries(engine_core PRIVATE SDL2::SDL2)
    target_compile_definitions(engine_core PRIVATE NM_HAS_SDL2)
endif()

# Define version
target_compile_definitions(engine_core
    PUBLIC
        NM_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        NM_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        NM_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)
