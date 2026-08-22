#include "app/Game.h"
#include "app/App.h"

#include "raylib.h"

#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace {


match3::App* g_app = nullptr;

void mainLoop() {
    g_app->tick();
}

} // namespace

int main() {

    // НАСТРОКИ ИГРЫ
    constexpr std::size_t cell_amount_x = 8;
    constexpr std::size_t cell_amount_y = 8;
    constexpr int cell_size = 64;
    constexpr int space_near_cell = 4; //то же самое что расстояние между клетками/2


    // Расчет размеров окна -------------
    //int x_offset = 44;
    //int y_offset = 64;
    //const int screenWidth =
    //    static_cast<int>(cell_amount_x) * cell_size + x_offset*2;

    //const int screenHeight =
    //    static_cast<int>(cell_amount_y) * cell_size + y_offset*2;
    //------------------------------------------

    constexpr int screenWidth = 800;
    constexpr int screenHeight = 800;

#ifndef __EMSCRIPTEN__
    // Desktop window may be resized by the user. In the web build we keep
    // the logical render size fixed so browser resizing does not distort
    // or rescale the game canvas through GLFW.
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
#endif

    InitWindow(screenWidth, screenHeight, "Match-3");
    SetTargetFPS(30);

    match3::App app(screenWidth, screenHeight);
    g_app = &app;

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
