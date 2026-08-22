#include "app/Game.h"
#include "app/App.h"

#include "raylib.h"

#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace {

//bool validate_settings(int cell_amount_x, int cell_amount_y, int cell_size, int space_near_cell ) {
//    if (cell_amount_x < 8 || cell_amount_x > 16) {
//        return false;
//    }
//    if (cell_amount_y < 8 || cell_amount_y > 16) {
//        return false;
//    }
//    if (cell_size < 32 || cell_size > 64) {
//        return false;
//    }
//    if (space_near_cell < 0 || space_near_cell > 8) {
//        return false;
//    }
//    return true;
//}

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

    //ПРОВЕРКА НАСТРОЕК
    //if (!validate_settings(cell_amount_x, cell_amount_y, cell_size, space_near_cell)) {
    //    std::cout << "Settings error" << std::endl;
    //    return 0;
    //}

    // Расчет размеров окна
    int x_offset = 44;
    int y_offset = 64;
    const int screenWidth =
        static_cast<int>(cell_amount_x) * cell_size + x_offset*2;

    const int screenHeight =
        static_cast<int>(cell_amount_y) * cell_size + y_offset*2;

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
