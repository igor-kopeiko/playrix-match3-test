# Match-3 Web (C++20)

Small C++ Match-3 project targeting Windows desktop and WebAssembly.

## Stack

- C++20
- CMake 3.24+
- raylib 6.0 — rendering/input/windowing
- Catch2 3.15.2 — unit tests for game logic
- Emscripten — WebAssembly/HTML5 build

Dependencies are fetched by CMake with `FetchContent`; nothing needs to be vendored into the repository.

## Project layout

```text
src/core/     Pure game logic; no raylib dependency
src/app/      Rendering, input, main loop
 tests/       Unit tests
 assets/      Game assets
 cmake/       Web shell and CMake helpers
 scripts/     Convenience scripts
```

## Windows / Visual Studio 2022

From Developer PowerShell:

```powershell
cmake --preset vs2022-debug
cmake --build --preset vs2022-debug
ctest --preset vs2022-debug
```

You can also open the repository folder directly in Visual Studio 2022; Visual Studio understands CMake projects.

## Ninja build

```powershell
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug
```

## Web build

Install and activate the Emscripten SDK first. Then install Ninja and run:

```powershell
./scripts/build-web.ps1
./scripts/serve-web.ps1
```

Open `http://localhost:8000/match3.html`.

## Architecture rule

`match3_core` must stay renderer-agnostic. Matching, swaps, cascades, gravity, scoring, objectives and RNG belong there. `src/app` should translate user input to core commands and render the resulting state.
