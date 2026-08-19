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
    switch (current_game_state) {
    case GameState::Idle:
        update_idle();
        break;

    case GameState::Swapping:
        update_swapping();
        break;
        
    case GameState::BackSwapping:
        update_back_swapping();
        break;

    case GameState::Removing:
        update_removing();
        break;

    case GameState::Falling:
        update_falling();
        break;

    case GameState::Filling:
        break;
    }

    //обработка нажатий мыши
    //if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    //    Vector2 mouse = GetMousePosition();

    //    // здесь обрабатываем клик
    //    const std::optional<Position> cell = get_chosen_cell(mouse);
    //    if (cell) {
    //        if (first_selected_cell) { //если первую клетку уже присвоили
    //            std::optional<Position> second_selected_cell = cell;
    //            if (board_.try_match_swap(first_selected_cell.value(), second_selected_cell.value())) {
    //                //найдем клетки которые входят в матч
    //                std::vector<Position> cells_in_match;
    //                while(1){
    //                    cells_in_match = board_.find_matches();
    //                    if (cells_in_match.empty()) {
    //                        break;
    //                    }

    //                    board_.delete_cells(cells_in_match);

    //                    //сдвинем остальные клетки
    //                    board_.collapse_cells();

    //                    //заполним пустоты
    //                    board_.fill_empty_cells();
    //                }

    //                
    //            }
    //            //board_.try_swap(first_selected_cell.value(), second_selected_cell.value());
    //            first_selected_cell = std::nullopt;
    //        }
    //        else {
    //            //присваиваем первую клетку
    //            first_selected_cell = cell;
    //        }
    //    }
    //}
}

void Game::update_idle() {
    //обработка нажатий мыши
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();

        // здесь обрабатываем клик
        const std::optional<Position> cell = get_chosen_cell(mouse);
        if (cell) {
            if (first_selected_cell) { //если первую клетку уже присвоили
                if (board_.are_neighbours(first_selected_cell.value(), cell.value())) {
                    //игра покажет анимацию смены, даже если смену делать нельзя
                    current_game_state = GameState::Swapping; //выставляем состояние
                    swapping_poses.first = first_selected_cell.value();//записываем позиции для анимации
                    swapping_poses.second = cell.value();
                    calculate_swapping_shift();
                }
                first_selected_cell = std::nullopt;
            }
            else {
                //присваиваем первую клетку
                first_selected_cell = cell;
            }
        }
    }
}

void Game::update_swapping() {

    animation_timer += GetFrameTime();

    if (animation_timer >= animation_duration.swapping) {
        
        //проверяем законной ли была смена
        if (board_.try_match_swap(swapping_poses.first, swapping_poses.second)) {
            animation_timer = 0.0; //сброс таймера только если нет back swapping
            current_game_state = GameState::Removing;

            //вычислим клетки для удаления
            //найдем клетки которые входят в матч
            cells_to_remove = board_.find_matches();
        }
        else {
            //значит нужно вернуть все обратно
            current_game_state = GameState::BackSwapping;
        }
    }
}

void Game::update_back_swapping() {
    //этот метод работает по принципу уменьшения таймера
    animation_timer -= GetFrameTime();

    if (animation_timer <= 0.0) {
        animation_timer = 0.0; //сброс таймера
        current_game_state = GameState::Idle;
    }
}

void Game::update_removing() {

    animation_timer += GetFrameTime();

    if (animation_timer >= animation_duration.removing) {
        animation_timer = 0.0; //сброс таймера
        board_.delete_cells(cells_to_remove); //окончательное удаление клеток
        current_game_state = GameState::Falling;

        //получим список клеток которые упали
        falling_cells = board_.collapse_cells();
    }
}

void Game::update_falling() {
    animation_timer += GetFrameTime();
    if (old_falling_complete) { //ждем пока анимация завершится
        animation_timer = 0.0;
        old_falling_complete = false;
        current_game_state = GameState::Filling;
    }
}

//==========================================================

void Game::draw() {
    BeginDrawing();
    //черный цвет
    ClearBackground(Color{24, 27, 36, 255});
    //заголовок: текст, x, y, размер шрифта
    //DrawText("Match-3 C++ / WebAssembly", kBoardOffsetX, 28, 24, RAYWHITE);
    int falling_cells_animation_complete_amount = 0;
    for (std::size_t y = 0; y < board_.height(); ++y) {
        for (std::size_t x = 0; x < board_.width(); ++x) {


            switch (current_game_state) {
            case GameState::Idle:
                //обычный показ доски
                draw_regular_cell(x, y);
                break;

            case GameState::Swapping:
                if (swapping_poses.first.x == x && swapping_poses.first.y == y) {
                    draw_swapping_cell(x, y);
                }
                else if (swapping_poses.second.x == x && swapping_poses.second.y == y) {
                    draw_swapping_cell(x, y);
                }
                else {
                    draw_regular_cell(x, y);
                }
                break;
            case GameState::BackSwapping:
                if (swapping_poses.first.x == x && swapping_poses.first.y == y) {
                    draw_swapping_cell(x, y);
                }
                else if (swapping_poses.second.x == x && swapping_poses.second.y == y) {
                    draw_swapping_cell(x, y);
                }
                else {
                    draw_regular_cell(x, y);
                }
                break;

            case GameState::Removing: {
                bool need_to_remove = false;
                for (auto& cell : cells_to_remove) {
                    if (cell.x == x && cell.y == y) {
                        need_to_remove = true;
                    }
                }
                if (need_to_remove) {
                    draw_removing_cell(x, y);
                }
                else {
                    draw_regular_cell(x, y);
                }
                break;
            }
            case GameState::Falling: {
                bool is_falling = false;
                for (auto& cell : falling_cells) {
                    if (cell.to.x == x && cell.to.y == y) {
                        is_falling = true;
                        if (draw_fallen_cell(cell)) {
                            falling_cells_animation_complete_amount++;
                        }
                    }
                }
                if (!is_falling) {
                    draw_regular_cell(x, y);
                }
                if (falling_cells_animation_complete_amount == falling_cells.size()) {
                    old_falling_complete = true;
                }
                break;
            }
            case GameState::Filling:
                draw_regular_cell(x, y);//DEBAG 
                break;

            } //switch



        }
    }

    EndDrawing();
}

void Game::draw_regular_cell(std::size_t x, std::size_t y) const {
    const int px = kBoardOffsetX + static_cast<int>(x) * kCellSize;
    const int py = kBoardOffsetY + static_cast<int>(y) * kCellSize;

    const Rectangle cell{
        static_cast<float>(px + space_near_cell),
        static_cast<float>(py + space_near_cell),
        static_cast<float>(kCellSize - space_near_cell * 2),
        static_cast<float>(kCellSize - space_near_cell * 2),
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

void Game::draw_swapping_cell(std::size_t x, std::size_t y) const {
    const int px = kBoardOffsetX + static_cast<int>(x) * kCellSize;
    const int py = kBoardOffsetY + static_cast<int>(y) * kCellSize;

    int shift_px = 0;
    int shift_py = 0;

    float progress = animation_timer / animation_duration.swapping;

    if (x == swapping_poses.first.x && y == swapping_poses.first.y) { //если это первая клетка
        shift_px = progress * (float)kCellSize * swapping_shift.x;
        shift_py = progress * (float)kCellSize * swapping_shift.y;
    }
    else {
        //здесь добавляем -1 потому что в противоположную сторону
        shift_px = progress * (float)kCellSize * swapping_shift.x * -1;
        shift_py = progress * (float)kCellSize * swapping_shift.y * -1;
    }
    const Rectangle cell{
        static_cast<float>(px + space_near_cell + shift_px),
        static_cast<float>(py + space_near_cell + shift_py),
        static_cast<float>(kCellSize - space_near_cell * 2),
        static_cast<float>(kCellSize - space_near_cell * 2),
    };
    DrawRectangleRounded(cell, 0.28F, 8, colorForTile(board_.at(x, y)));
}

void Game::draw_removing_cell(std::size_t x, std::size_t y) const {
    //Делает прозрачность
    const int px = kBoardOffsetX + static_cast<int>(x) * kCellSize;
    const int py = kBoardOffsetY + static_cast<int>(y) * kCellSize;

    int shift_px = 0;
    int shift_py = 0;

    float progress = animation_timer / animation_duration.removing;

    const Rectangle cell{
        static_cast<float>(px + space_near_cell + shift_px),
        static_cast<float>(py + space_near_cell + shift_py),
        static_cast<float>(kCellSize - space_near_cell * 2),
        static_cast<float>(kCellSize - space_near_cell * 2),
    };
    Color color = Fade(colorForTile(board_.at(x, y)), 1.0 - progress); //прозрачность
    DrawRectangleRounded(cell, 0.28F, 8, color);
}

bool Game::draw_fallen_cell(FallMove fall_cell) const{
    const int px = kBoardOffsetX + static_cast<int>(fall_cell.from.x) * kCellSize;
    int py = 0;

    const int py_start = kBoardOffsetY + static_cast<int>(fall_cell.from.y) * kCellSize;
    const int py_end = kBoardOffsetY + static_cast<int>(fall_cell.to.y) * kCellSize;


    bool is_animation_complete = false;
    // *animation_duration.fall_speed_pix_pro_sec;
    int py_shift = animation_timer * animation_duration.fall_speed_pix_pro_sec;
    //std::cout << "animation_timer = " << animation_timer << std::endl;
    //std::cout << " animation_duration.fall_speed_pix_pro_sec = " << animation_duration.fall_speed_pix_pro_sec << std::endl;
    //std::cout << "py_shift = " << py_shift << std::endl;

    if (py_start + py_shift >= py_end) {
        is_animation_complete = true;
        py = py_end;
    }
    else {
        py = py_start + py_shift;
    }

    //std::cout << "fall: x = " << fall_cell.to.x << " y = " << fall_cell.to.y << " py = "<< py << std::endl;

    const Rectangle cell{
        static_cast<float>(px + space_near_cell),
        static_cast<float>(py + space_near_cell),
        static_cast<float>(kCellSize - space_near_cell * 2),
        static_cast<float>(kCellSize - space_near_cell * 2),
    };
    DrawRectangleRounded(cell, 0.28F, 8, colorForTile(fall_cell.tile));
    return is_animation_complete;
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

void Game::calculate_swapping_shift() {
    if (swapping_poses.first.x == swapping_poses.second.x) {
        if (swapping_poses.first.y > swapping_poses.second.y) {
            //значит сдвиг первого вверх
            swapping_shift = {0, -1};
        }
        else {
            //сдвиг первого вниз
            swapping_shift = {0, 1};
        }
    }
    else {
        if (swapping_poses.first.x > swapping_poses.second.x) {
            //сдвиг первого влево
            swapping_shift = { -1, 0 };
        }
        else {
            //сдвиг первого вправо
            swapping_shift = { 1, 0 };
        }
    }
    
}


} // namespace match3
