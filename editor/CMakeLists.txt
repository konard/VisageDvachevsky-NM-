# NovelMind Visual Editor

add_library(novelmind_editor STATIC
    src/editor_app.cpp
    src/editor_runtime_host.cpp
    src/diagnostics_panel.cpp
    src/asset_pipeline.cpp
    src/editor_settings.cpp
)

# Alias for backward compatibility
add_library(editor ALIAS novelmind_editor)

target_include_directories(novelmind_editor
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(novelmind_editor
    PUBLIC
        engine_core
    PRIVATE
        novelmind_compiler_options
)

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
