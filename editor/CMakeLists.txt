# NovelMind Visual Editor

add_library(editor STATIC
    src/editor_app.cpp
)

target_include_directories(editor
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(editor
    PUBLIC
        engine_core
    PRIVATE
        novelmind_compiler_options
)

# Editor executable (optional, only build if main.cpp exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
    add_executable(novelmind_editor
        src/main.cpp
    )

    target_link_libraries(novelmind_editor
        PRIVATE
            editor
            engine_core
            novelmind_compiler_options
    )
endif()
