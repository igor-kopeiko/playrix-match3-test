#include "app/Game.h"

#include "raylib.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace {

match3::Game* g_game = nullptr;

void mainLoop() {
    g_game->tick();
}

} // namespace

int main() {
    constexpr int screenWidth = 600;
    constexpr int screenHeight = 640;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Match-3");
    SetTargetFPS(60);

    match3::Game game;
    g_game = &game;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    while (!WindowShouldClose()) {
        mainLoop();
    }
#endif

    CloseWindow();
    return 0;
}
