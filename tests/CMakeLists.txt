# Tests configuration
include(FetchContent)

# Fetch Catch2 for testing
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# Unit tests
add_executable(unit_tests
    unit/test_result.cpp
    unit/test_timer.cpp
    unit/test_memory_fs.cpp
    unit/test_vm.cpp
    unit/test_value.cpp
    unit/test_lexer.cpp
    unit/test_parser.cpp
)

target_link_libraries(unit_tests
    PRIVATE
        engine_core
        Catch2::Catch2WithMain
        novelmind_compiler_options
)

# Register tests
include(CTest)
include(Catch)
catch_discover_tests(unit_tests)
