#include "app/Game.h"

#include "raylib.h"

#include <array>

namespace match3 {
namespace {




Color colorForTile(const Tile tile) {
    constexpr std::array<Color, static_cast<std::size_t>(Tile::Count)> colors{
        Color{230, 70, 70, 255},
        Color{80, 190, 100, 255},
        Color{70, 120, 235, 255},
        Color{245, 205, 65, 255},
        Color{160, 90, 220, 255},
        Color{245, 145, 55, 255},
    };
    return colors.at(static_cast<std::size_t>(tile));
}

} // namespace

//Game::Game() = default;
Game::Game(int kBoardOffsetX, int kBoardOffsetY, int kCellSize, int space_near_cell, std::size_t cell_amount_x, std::size_t cell_amount_y)
    : kBoardOffsetX{ kBoardOffsetX },
    kBoardOffsetY{ kBoardOffsetY },
    kCellSize{ kCellSize },
    space_near_cell{ space_near_cell },
    cell_amount_x{ cell_amount_x },
    cell_amount_y{ cell_amount_y }
{

}


void Game::tick() {
    update();
    draw();
}

void Game::update() {
    // Input and animation state will live here.
    
    //обработка нажатий мыши
    //if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    //    Vector2 mouse = GetMousePosition();

    //    // здесь обрабатываем клик
    //}
}

void Game::draw() const {
    BeginDrawing();
    //черный цвет
    ClearBackground(Color{24, 27, 36, 255});
    //заголовок: текст, x, y, размер шрифта
    //DrawText("Match-3 C++ / WebAssembly", kBoardOffsetX, 28, 24, RAYWHITE);

    for (std::size_t y = 0; y < board_.height(); ++y) {
        for (std::size_t x = 0; x < board_.width(); ++x) {
            const int px = kBoardOffsetX + static_cast<int>(x) * kCellSize;
            const int py = kBoardOffsetY + static_cast<int>(y) * kCellSize;
            const Rectangle cell{
                static_cast<float>(px + space_near_cell),
                static_cast<float>(py + space_near_cell),
                static_cast<float>(kCellSize - space_near_cell*2),
                static_cast<float>(kCellSize - space_near_cell*2),
            };

            DrawRectangleRounded(cell, 0.28F, 8, colorForTile(board_.at(x, y)));
        }
    }

    EndDrawing();
}


//=============================================
//Position Game::get_chosen_cell(Vector2 mouse) {
//
//    Position pos;
//
//    return pos;
//}


} // namespace match3
