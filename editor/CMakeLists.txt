# NovelMind Visual Editor

add_library(novelmind_editor STATIC
    # Core editor
    src/editor_app.cpp
    src/editor_runtime_host.cpp
    src/diagnostics_panel.cpp
    src/asset_pipeline.cpp
    src/editor_settings.cpp
    src/voice_manager.cpp
    src/timeline_editor.cpp
    src/curve_editor.cpp

    # v0.2.0 backend systems
    src/sdl_imgui_backend.cpp
    src/selection_system.cpp
    src/event_bus.cpp
    src/project_manager.cpp
    src/inspector_binding.cpp
    src/asset_preview.cpp
    src/timeline_playback.cpp
    src/editor_state.cpp
    src/error_reporter.cpp

    # v0.2.0 GUI systems
    src/imgui_integration.cpp
    src/gui_panel_base.cpp
    src/scene_view_panel.cpp
    src/story_graph_panel.cpp
    src/timeline_panel.cpp
    src/inspector_panel.cpp
    src/asset_browser_panel.cpp
    src/hierarchy_panel.cpp
    src/console_panel.cpp
    src/build_settings_panel.cpp
    src/play_mode_controller.cpp

    # v0.2.0 Additional GUI panels
    src/curve_editor_panel.cpp
    src/voice_manager_panel.cpp
    src/localization_panel.cpp
    src/debug_overlay_panel.cpp
    src/hotkeys_manager.cpp
)

# Alias for backward compatibility
add_library(editor ALIAS novelmind_editor)

target_include_directories(novelmind_editor
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Optional dependencies for GUI
find_package(SDL2 QUIET)
find_package(OpenGL QUIET)

# Check for ImGui (can be provided via FetchContent or system install)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../external/imgui/imgui.h")
    set(IMGUI_FOUND TRUE)
    set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../external/imgui")
endif()

target_link_libraries(novelmind_editor
    PUBLIC
        engine_core
    PRIVATE
        novelmind_compiler_options
)

# Configure SDL2 and ImGui if available
if(SDL2_FOUND AND IMGUI_FOUND AND OPENGL_FOUND)
    target_compile_definitions(novelmind_editor PRIVATE
        NOVELMIND_HAS_SDL2
        NOVELMIND_HAS_IMGUI
    )
    target_include_directories(novelmind_editor PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${OPENGL_INCLUDE_DIR}
    )
    target_link_libraries(novelmind_editor PRIVATE
        ${SDL2_LIBRARIES}
        ${OPENGL_LIBRARIES}
    )
    message(STATUS "NovelMind Editor: SDL2 + ImGui GUI enabled")
elseif(SDL2_FOUND)
    target_compile_definitions(novelmind_editor PRIVATE NOVELMIND_HAS_SDL2)
    target_include_directories(novelmind_editor PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(novelmind_editor PRIVATE ${SDL2_LIBRARIES})
    message(STATUS "NovelMind Editor: SDL2 available but ImGui not found - limited GUI")
else()
    message(STATUS "NovelMind Editor: Building without GUI (SDL2/ImGui not found)")
endif()

# Editor executable (optional, only build if main.cpp exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_executable(novelmind_editor_app
        src/main.cpp
    )

    target_link_libraries(novelmind_editor_app
        PRIVATE
            novelmind_editor
            engine_core
            novelmind_compiler_options
    )

    # Set output name to novelmind_editor
    set_target_properties(novelmind_editor_app PROPERTIES
        OUTPUT_NAME "novelmind_editor"
    )
endif()
