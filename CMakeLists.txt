cmake_minimum_required(VERSION 3.24)

project(match3_web
    VERSION 0.1.0
    DESCRIPTION "Match-3 game in C++ for desktop and WebAssembly"
    LANGUAGES CXX
)

option(MATCH3_BUILD_TESTS "Build unit tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# -------------------- Dependencies --------------------
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

# -------------------- Core game logic --------------------
add_library(match3_core STATIC
    src/core/Board.cpp
)

target_include_directories(match3_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(match3_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(match3_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(match3_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# -------------------- Application --------------------
add_executable(match3
    src/app/main.cpp
    src/app/Game.cpp
    src/app/App.cpp
)

target_link_libraries(match3 PRIVATE match3_core raylib)
target_include_directories(match3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(EMSCRIPTEN)
    set_target_properties(match3 PROPERTIES SUFFIX ".html")
    target_link_options(match3 PRIVATE
        "-sUSE_GLFW=3"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sASSERTIONS=1"
        "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/cmake/shell.html"
    )
endif()

# -------------------- Tests --------------------
if(MATCH3_BUILD_TESTS AND NOT EMSCRIPTEN)
    enable_testing()

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.15.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(match3_tests
        tests/BoardTests.cpp
    )
    target_link_libraries(match3_tests PRIVATE match3_core Catch2::Catch2WithMain)

    include(Catch)
    catch_discover_tests(match3_tests)
endif()
