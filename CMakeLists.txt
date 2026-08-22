cmake_minimum_required(VERSION 3.24)

project(match3_web
    VERSION 0.1.0
    DESCRIPTION "Match-3 game in C++ for desktop and WebAssembly"
    LANGUAGES CXX
)

option(MATCH3_BUILD_TESTS "Build unit tests" ON)
option(MATCH3_ENABLE_ASAN "Enable AddressSanitizer" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

function(match3_enable_asan target_name)
    if(MATCH3_ENABLE_ASAN AND MSVC AND NOT EMSCRIPTEN)
        target_compile_options(${target_name} PRIVATE /fsanitize=address)
        target_link_options(${target_name} PRIVATE /fsanitize=address)
    endif()
endfunction()

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

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

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

match3_enable_asan(match3_core)

# -------------------- Application --------------------
add_executable(match3
    src/app/main.cpp
    src/app/Game.cpp
    src/app/App.cpp
)

target_link_libraries(match3 PRIVATE
    match3_core
    raylib
    nlohmann_json::nlohmann_json
)
target_include_directories(match3 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

match3_enable_asan(match3)

# Desktop development builds read assets directly from the source tree.
# This allows level JSON files to be hot-reloaded without copying them.
if(NOT EMSCRIPTEN)
    target_compile_definitions(match3 PRIVATE
        MATCH3_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
    )
endif()

if(EMSCRIPTEN)
    # The browser cannot access the host filesystem. Emscripten packages the
    # repository assets into its virtual filesystem at /assets.
    target_compile_definitions(match3 PRIVATE
        MATCH3_ASSETS_DIR="/assets"
    )

    # Produce a static-site bundle directly in dist/ so it can be published
    # by Cloudflare Pages or any other static hosting provider.
    set(MATCH3_WEB_DIST_DIR "${CMAKE_SOURCE_DIR}/dist")
    file(MAKE_DIRECTORY "${MATCH3_WEB_DIST_DIR}")

    set_target_properties(match3 PROPERTIES
        SUFFIX ".html"
        OUTPUT_NAME "index"
        RUNTIME_OUTPUT_DIRECTORY "${MATCH3_WEB_DIST_DIR}"
    )

    target_link_options(match3 PRIVATE
        "-sUSE_GLFW=3"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sASSERTIONS=0"
        "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/cmake/shell.html"
        "--preload-file"
        "${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
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

    match3_enable_asan(match3_tests)

    include(Catch)
    catch_discover_tests(match3_tests)
endif()
