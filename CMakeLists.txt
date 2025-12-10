cmake_minimum_required(VERSION 3.20)

project(NovelMind
    VERSION 0.1.0
    DESCRIPTION "A modern 2D visual novel engine"
    LANGUAGES CXX
)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Options
option(NOVELMIND_BUILD_TESTS "Build unit tests" ON)
option(NOVELMIND_BUILD_EDITOR "Build visual editor" OFF)
option(NOVELMIND_ENABLE_ASAN "Enable AddressSanitizer" OFF)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compiler warnings
add_library(novelmind_compiler_options INTERFACE)

target_compile_options(novelmind_compiler_options INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wcast-qual
        -Wformat=2
        -Wundef
        -Werror=return-type
        -Wshadow
        -Wcast-align
        -Wunused
        -Wnull-dereference
        -Wdouble-promotion
        -Wimplicit-fallthrough
    >
    $<$<CXX_COMPILER_ID:GNU>:
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        /permissive-
        /Zc:__cplusplus
    >
)

# AddressSanitizer
if(NOVELMIND_ENABLE_ASAN)
    target_compile_options(novelmind_compiler_options INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
            -fsanitize=address
            -fno-omit-frame-pointer
        >
    )
    target_link_options(novelmind_compiler_options INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
            -fsanitize=address
        >
    )
endif()

# Engine core library
add_subdirectory(engine_core)

# Tests
if(NOVELMIND_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Editor (optional)
if(NOVELMIND_BUILD_EDITOR)
    add_subdirectory(editor)
endif()

# Compiler (always build for testing)
add_subdirectory(compiler)

# Installation
include(GNUInstallDirs)
install(TARGETS engine_core
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY engine_core/include/NovelMind
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
