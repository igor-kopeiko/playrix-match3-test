#include "app/Game.h"

#include "raylib.h"

#include <array>

namespace match3 {
namespace {




Color colorForTile(const Tile tile) {
    constexpr std::array<Color, static_cast<std::size_t>(Tile::Count)> colors{
        Color{24, 27, 36, 255}, //default
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
    // 
    //std::vector<Position> matched_cells = board_.find_matches();
    //for (auto& elem : matched_cells) {
    //    std::cout << " [" << elem.x << " " << elem.y << "] ";
    //}
    //std::cout << std::endl;
    // 
    
    //std::cout << "possible = " << board_.has_possible_moves() << std::endl;
    
    //обработка нажатий мыши
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();

        // здесь обрабатываем клик
        const std::optional<Position> cell = get_chosen_cell(mouse);
        if (cell) {
            if (first_selected_cell) { //если первую клетку уже присвоили
                std::optional<Position> second_selected_cell = cell;
                if (board_.try_match_swap(first_selected_cell.value(), second_selected_cell.value())) {
                    //найдем клетки которые входят в матч
                    std::vector<Position> cells_in_match = board_.find_matches();
                    board_.delete_cells(cells_in_match);
                }
                //board_.try_swap(first_selected_cell.value(), second_selected_cell.value());
                first_selected_cell = std::nullopt;
            }
            else {
                //присваиваем первую клетку
                first_selected_cell = cell;
            }
        }
    }
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
            if (first_selected_cell) { //если выбрана клетка
                if (first_selected_cell->x == x && first_selected_cell->y == y) { //если это текущая отрисовка
                    const Rectangle border_cell{
                        static_cast<float>(px),
                        static_cast<float>(py),
                        static_cast<float>(kCellSize),
                        static_cast<float>(kCellSize),
                    };
                    
                    DrawRectangleRounded(border_cell, 0.28F, 8, WHITE);
                }
            }
            
            DrawRectangleRounded(cell, 0.28F, 8, colorForTile(board_.at(x, y)));
        }
    }

    EndDrawing();
}


//=============================================
std::optional<Position> Game::get_chosen_cell(Vector2 mouse) {

    //считаем входит ли мышка на поле
    int right_border = kBoardOffsetX + cell_amount_x * kCellSize;
    if (mouse.x < kBoardOffsetX || mouse.x >= right_border) {
        std::cout << "Clicked missed" << std::endl;
        return std::nullopt;
    }
    int lower_border = kBoardOffsetY + cell_amount_y * kCellSize;
    if (mouse.y < kBoardOffsetY || mouse.y >= lower_border) {
        std::cout << "Clicked missed" << std::endl;
        return std::nullopt;
    }

    
    std::size_t local_mouse_x = mouse.x - kBoardOffsetX;
    std::size_t local_mouse_y = mouse.y - kBoardOffsetY;

    std::size_t x = local_mouse_x / kCellSize;
    std::size_t y = local_mouse_y / kCellSize;

    std::cout << "Clicked on border: x = " << x << " y = " << y << std::endl;

    return Position{ x, y };
}


} // namespace match3
