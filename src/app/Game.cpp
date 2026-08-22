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


Game::Game(LevelConfig level_config, int screenWidth, int screenHeight)
    : level_config{ level_config },
    screenWidth{ screenWidth },
    screenHeight{ screenHeight },
    board_{ (std::size_t)level_config.width, (std::size_t)level_config.height, level_config.colors_count, 42 }
{

    std::cout << "Game created" << std::endl;

    animation_duration.swapping = 0.2;
    animation_duration.removing = 0.2;
    animation_duration.fall_speed_pix_pro_sec = 500.0;

    //Game сам вычисляет :
    //    -cell_size
    //    - board_offset_x
    //    - board_offset_y

    kBoardOffsetX = 44; //отступ первой ячейки от края
    kBoardOffsetY = 64;

    kCellSize_x = (screenWidth - kBoardOffsetX * 2) / level_config.width;

    kCellSize_y = (screenHeight - kBoardOffsetY * 2) / level_config.height;
    space_near_cell = 4; //то же самое что расстояние между клетками/2

    //Загрузка текстур
    std::string tiles_path = MATCH3_ASSETS_DIR;
    tiles_path += "/tiles/";

    tile_textures_[static_cast<std::size_t>(Tile::Red)] =
        LoadTexture((tiles_path + "red_60.png").c_str());

    tile_textures_[static_cast<std::size_t>(Tile::Green)] =
        LoadTexture((tiles_path + "green_60.png").c_str());

    tile_textures_[static_cast<std::size_t>(Tile::Blue)] =
        LoadTexture((tiles_path + "blue_60.png").c_str());

    tile_textures_[static_cast<std::size_t>(Tile::Yellow)] =
        LoadTexture((tiles_path + "yellow_60.png").c_str());

    tile_textures_[static_cast<std::size_t>(Tile::Purple)] =
        LoadTexture((tiles_path + "purple_60.png").c_str());

    tile_textures_[static_cast<std::size_t>(Tile::Orange)] =
        LoadTexture((tiles_path + "orange_60.png").c_str());
}


void Game::tick() {
    update();
    draw();
}

GameAction Game::requested_action() const {
    return game_action_;
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

    case GameState::Checking:
        update_checking();
        break;

    case GameState::Shuffling:
        update_shuffling();
        break;

    case GameState::GameOver:
        update_game_over();
        break;
    }
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
            level_config.moves--;

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
        //записываем сколько каких цветов удалили
        calculate_deleted_targets();
        board_.delete_cells(cells_to_remove); //окончательное удаление клеток
        current_game_state = GameState::Falling;

        //получим список клеток которые упали
        falling_cells = board_.collapse_cells();
        std::vector<FallMove> new_cells = board_.fill_empty_cells();

        falling_cells.insert(
            falling_cells.end(),
            new_cells.begin(),
            new_cells.end()
        );

        //Вычислим самый долгое по времени падение
        int max_dist = 0;
        for (auto& elem : falling_cells) {
            int py_start = kBoardOffsetY + static_cast<int>(elem.from.y) * kCellSize_y;
            int py_end = kBoardOffsetY + static_cast<int>(elem.to.y) * kCellSize_y;
            int dist = py_end - py_start;
            if (dist > max_dist) {
                max_dist = dist;
            }
        }
        max_falling_time = (float)max_dist / animation_duration.fall_speed_pix_pro_sec;

    }
}

void Game::update_falling() {

    animation_timer += GetFrameTime();
    if (animation_timer >= max_falling_time) { //ждем пока анимация завершится
        animation_timer = 0.0;
        current_game_state = GameState::Checking;
    }
}

void Game::update_checking() {
    cells_to_remove = board_.find_matches();
    if (cells_to_remove.empty()) {
        if (board_.has_possible_moves()) {

            //проверяем завершение
            if (check_game_over()) {
                //завершаем игру
                current_game_state = GameState::GameOver;
            }
            else {
                current_game_state = GameState::Idle;
            }
        }
        else {
            current_game_state = GameState::Shuffling;
        }
        
    }
    else {
        current_game_state = GameState::Removing;
    }
}

void Game::update_shuffling() {
    board_.shuffle_until_possible_solution();
    current_game_state = GameState::Idle;
}

void Game::update_game_over() {
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const Vector2 mouse = GetMousePosition();

        if (CheckCollisionPointRec(mouse, get_exit_button_rect())) {
            game_action_ = GameAction::ExitToMenu;
        }
    }


}

Rectangle Game::get_exit_button_rect() const {
    constexpr float width = 180.0f;
    constexpr float height = 45.0f;

    return Rectangle{
        (screenWidth - width) / 2.0f,
        screenHeight / 2.0f + 40.0f,
        width,
        height
    };
}


//==========================================================

void Game::draw() const {
    BeginDrawing();
    //черный цвет
    ClearBackground(Color{24, 27, 36, 255});

    BeginScissorMode(
        kBoardOffsetX,
        kBoardOffsetY,
        static_cast<int>(board_.width()) * kCellSize_x,
        static_cast<int>(board_.height()) * kCellSize_y
    );
    for (std::size_t y = 0; y < board_.height(); ++y) {
        for (std::size_t x = 0; x < board_.width(); ++x) {


            switch (current_game_state) {
            case GameState::Idle:
                //DrawText("State: Idle", kBoardOffsetX, 28, 24, RAYWHITE);
                //обычный показ доски
                draw_regular_cell(x, y);
                break;

            case GameState::Swapping:
                //("State: Swapping", kBoardOffsetX, 28, 24, RAYWHITE);
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
                //DrawText("State: BackSwapping", kBoardOffsetX, 28, 24, RAYWHITE);
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
                //DrawText("State: Removing", kBoardOffsetX, 28, 24, RAYWHITE);
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
                //DrawText("State: Falling", kBoardOffsetX, 28, 24, RAYWHITE);
                bool is_falling = false;
                for (auto& cell : falling_cells) {
                    if (cell.to.x == x && cell.to.y == y) {
                        is_falling = true;
                        draw_fallen_cell(cell);
                    }
                }
                if (!is_falling) {
                    draw_regular_cell(x, y);
                }
                break;
            }
            case GameState::Checking:
                //DrawText("State: Filling", kBoardOffsetX, 28, 24, RAYWHITE);
                draw_regular_cell(x, y);//DEBAG 
                break;

            case GameState::GameOver:
                draw_regular_cell(x, y);
                break;
            } //switch



        }
    }

    EndScissorMode();

    draw_goals();
    draw_move_amount();
    if (current_game_state == GameState::GameOver) {
        draw_game_over();
    }

    EndDrawing();
}

const Texture2D& Game::texture_for_tile(Tile tile) const
{
    return tile_textures_.at(static_cast<std::size_t>(tile));
}

void Game::draw_tile(
    Tile tile,
    const Rectangle& destination,
    Color tint
) const
{
    if (tile == Tile::Default) {
        return;
    }

    const Texture2D& texture = texture_for_tile(tile);

    const Rectangle source{
        0.0f,
        0.0f,
        static_cast<float>(texture.width),
        static_cast<float>(texture.height)
    };

    DrawTexturePro(
        texture,
        source,
        destination,
        Vector2{ 0.0f, 0.0f },
        0.0f,
        tint
    );
}

void Game::draw_regular_cell(std::size_t x, std::size_t y) const {
    const int px = kBoardOffsetX + static_cast<int>(x) * kCellSize_x;
    const int py = kBoardOffsetY + static_cast<int>(y) * kCellSize_y;

    const Rectangle cell{
        static_cast<float>(px + space_near_cell),
        static_cast<float>(py + space_near_cell),
        static_cast<float>(kCellSize_x - space_near_cell * 2),
        static_cast<float>(kCellSize_y - space_near_cell * 2),
    };
    if (first_selected_cell) { //если выбрана клетка
        if (first_selected_cell->x == x && first_selected_cell->y == y) { //если это текущая отрисовка
            const Rectangle border_cell{
                static_cast<float>(px),
                static_cast<float>(py),
                static_cast<float>(kCellSize_x),
                static_cast<float>(kCellSize_y),
            };

            DrawRectangleRounded(border_cell, 0.28F, 8, WHITE);
        }
    }
    //DrawRectangleRounded(cell, 0.28F, 8, colorForTile(board_.at(x, y))); //отрисовка просто цвета
    draw_tile(board_.at(x, y), cell); //отрисовка текстур
}

void Game::draw_swapping_cell(std::size_t x, std::size_t y) const {
    const int px = kBoardOffsetX + static_cast<int>(x) * kCellSize_x;
    const int py = kBoardOffsetY + static_cast<int>(y) * kCellSize_y;

    int shift_px = 0;
    int shift_py = 0;

    float progress = animation_timer / animation_duration.swapping;

    if (x == swapping_poses.first.x && y == swapping_poses.first.y) { //если это первая клетка
        shift_px = progress * (float)kCellSize_x * swapping_shift.x;
        shift_py = progress * (float)kCellSize_y * swapping_shift.y;
    }
    else {
        //здесь добавляем -1 потому что в противоположную сторону
        shift_px = progress * (float)kCellSize_x * swapping_shift.x * -1;
        shift_py = progress * (float)kCellSize_y * swapping_shift.y * -1;
    }
    const Rectangle cell{
        static_cast<float>(px + space_near_cell + shift_px),
        static_cast<float>(py + space_near_cell + shift_py),
        static_cast<float>(kCellSize_x - space_near_cell * 2),
        static_cast<float>(kCellSize_y - space_near_cell * 2),
    };
    //DrawRectangleRounded(cell, 0.28F, 8, colorForTile(board_.at(x, y))); //отрисовка цветов
    draw_tile(board_.at(x, y), cell); //отрисовка текстур
}

void Game::draw_removing_cell(std::size_t x, std::size_t y) const {
    //Делает прозрачность
    const int px = kBoardOffsetX + static_cast<int>(x) * kCellSize_x;
    const int py = kBoardOffsetY + static_cast<int>(y) * kCellSize_y;

    int shift_px = 0;
    int shift_py = 0;

    float progress = animation_timer / animation_duration.removing;

    const Rectangle cell{
        static_cast<float>(px + space_near_cell + shift_px),
        static_cast<float>(py + space_near_cell + shift_py),
        static_cast<float>(kCellSize_x - space_near_cell * 2),
        static_cast<float>(kCellSize_y - space_near_cell * 2),
    };
    //ОТРИСОВКА ЦВЕТОВ
    //Color color = Fade(colorForTile(board_.at(x, y)), 1.0 - progress); //прозрачность
    //DrawRectangleRounded(cell, 0.28F, 8, color);

    //ОТРИСОВКА ТЕКСТУР
    const Color tint = Fade(WHITE, 1.0f - progress);

    draw_tile(
        board_.at(x, y),
        cell,
        tint
    );
}

void Game::draw_fallen_cell(FallMove fall_cell) const{
    const int px = kBoardOffsetX + static_cast<int>(fall_cell.from.x) * kCellSize_x;
    int py = 0;

    const int py_start = kBoardOffsetY + static_cast<int>(fall_cell.from.y) * kCellSize_y;
    const int py_end = kBoardOffsetY + static_cast<int>(fall_cell.to.y) * kCellSize_y;

    bool is_animation_complete = false;
    int py_shift = animation_timer * animation_duration.fall_speed_pix_pro_sec;
    if (py_start + py_shift >= py_end) {
        is_animation_complete = true;
        py = py_end;
    }
    else {
        py = py_start + py_shift;
    }

    const Rectangle cell{
        static_cast<float>(px + space_near_cell),
        static_cast<float>(py + space_near_cell),
        static_cast<float>(kCellSize_x - space_near_cell * 2),
        static_cast<float>(kCellSize_y - space_near_cell * 2),
    };
    //DrawRectangleRounded(cell, 0.28F, 8, colorForTile(fall_cell.tile)); //цвета
    draw_tile(fall_cell.tile, cell); //текстуры
}

void Game::draw_game_over() const
{
    // Затемнение поверх игрового поля
    DrawRectangle(
        0,
        0,
        screenWidth,
        screenHeight,
        Color{ 0, 0, 0, 140 }
    );

    // Основная панель
    constexpr float panel_width = 320.0f;
    constexpr float panel_height = 220.0f;

    const float panel_x =
        (static_cast<float>(screenWidth) - panel_width) / 2.0f;

    const float panel_y =
        (static_cast<float>(screenHeight) - panel_height) / 2.0f;

    const Rectangle panel{
        panel_x,
        panel_y,
        panel_width,
        panel_height
    };

    DrawRectangleRounded(
        panel,
        0.15f,
        8,
        Color{ 40, 45, 60, 255 }
    );

    DrawRectangleRoundedLines(
        panel,
        0.15f,
        8,
        RAYWHITE
    );


    // -------------------------
    // Victory / Defeat
    // -------------------------

    const char* result_text = "";

    if (game_result_ == GameResult::Victory) {
        result_text = "Victory!";
    }
    else if (game_result_ == GameResult::Defeat) {
        result_text = "Defeat";
    }

    constexpr int result_font_size = 40;

    const int result_text_width =
        MeasureText(result_text, result_font_size);

    DrawText(
        result_text,
        (screenWidth - result_text_width) / 2,
        static_cast<int>(panel_y + 35),
        result_font_size,
        RAYWHITE
    );


    // -------------------------
    // Кнопка Level select
    // -------------------------

    const Rectangle button = get_exit_button_rect();

    const Vector2 mouse = GetMousePosition();

    const bool hovered =
        CheckCollisionPointRec(mouse, button);

    Color button_color{
        55, 62, 80, 255
    };

    if (hovered) {
        button_color = Color{
            75, 85, 110, 255
        };
    }

    DrawRectangleRounded(
        button,
        0.2f,
        8,
        button_color
    );

    DrawRectangleRoundedLines(
        button,
        0.2f,
        8,
        RAYWHITE
    );


    // -------------------------
    // Текст кнопки
    // -------------------------

    const char* button_text = "Level select";

    constexpr int button_font_size = 20;

    const int button_text_width =
        MeasureText(button_text, button_font_size);

    const int button_text_x =
        static_cast<int>(
            button.x +
            (button.width - button_text_width) / 2.0f
            );

    const int button_text_y =
        static_cast<int>(
            button.y +
            (button.height - button_font_size) / 2.0f
            );

    DrawText(
        button_text,
        button_text_x,
        button_text_y,
        button_font_size,
        RAYWHITE
    );
}


//=============================================
std::optional<Position> Game::get_chosen_cell(Vector2 mouse) {

    //считаем входит ли мышка на поле
    int right_border = kBoardOffsetX + level_config.width * kCellSize_x;
    if (mouse.x < kBoardOffsetX || mouse.x >= right_border) {
        return std::nullopt;
    }
    int lower_border = kBoardOffsetY + level_config.height * kCellSize_y;
    if (mouse.y < kBoardOffsetY || mouse.y >= lower_border) {
        return std::nullopt;
    }

    
    std::size_t local_mouse_x = mouse.x - kBoardOffsetX;
    std::size_t local_mouse_y = mouse.y - kBoardOffsetY;

    int x = local_mouse_x / kCellSize_x;
    int y = local_mouse_y / kCellSize_y;

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

void Game::draw_move_amount() const {
    int moves_left = level_config.moves;
    if (moves_left < 0) {
        moves_left = 0;
    }

    std::string moves_str = "Moves left: ";
    moves_str += std::to_string(moves_left);
    DrawText(moves_str.c_str(), 20 , screenHeight - 30, 24, RAYWHITE);
}

void Game::draw_goals() const
{
    const int start_x = 20;
    const int start_y = 20;

    const int font_size = 20;

    const int goal_box_size = 18;   // меньше обычной клетки
    const int gap_after_box = 6;
    const int gap_between_goals = 18;

    int current_x = start_x;

    // "Goals:"
    DrawText(
        "Goals:",
        current_x,
        start_y,
        font_size,
        RAYWHITE
    );

    current_x += MeasureText("Goals:", font_size) + 12;

    for (const auto& goal : level_config.goals) {

        const Rectangle color_box{
            static_cast<float>(current_x),
            static_cast<float>(start_y + 1),
            static_cast<float>(goal_box_size),
            static_cast<float>(goal_box_size)
        };

        DrawRectangleRounded(
            color_box,
            0.25f,
            6,
            colorForTile(goal.color)
        );

        current_x += goal_box_size + gap_after_box;

        int goal_amount = goal.amount;
        if (goal_amount < 0) {
            goal_amount = 0;
        }

        std::string amount_text = std::to_string(goal_amount);

        DrawText(
            amount_text.c_str(),
            current_x,
            start_y,
            font_size,
            RAYWHITE
        );

        current_x +=
            MeasureText(amount_text.c_str(), font_size)
            + gap_between_goals;
    }
}

void Game::calculate_deleted_targets(){
    for (const auto& pos : cells_to_remove) {
        //считаем какой цвет можно удалить
        Tile color_to_remove = board_.at(pos);
        for (auto& goal : level_config.goals) {
            if (goal.color == color_to_remove) {
                goal.amount--;
            }
        }
    }
}

bool Game::check_game_over() {
    //проверим что цели выполнены
    if (are_all_goals_completed()) {
        game_result_ = GameResult::Victory;
        return true;
    }

    //проверим что движения закончились
    if (level_config.moves <= 0) {
        game_result_ = GameResult::Defeat;
        return true;
    }

    return false;
}

bool Game::are_all_goals_completed() const {
    for (const auto& goal : level_config.goals) {
        if (goal.amount > 0) {
            return false;
        }
    }
    return true;
}

Game::~Game()
{
    for (auto& texture : tile_textures_) {
        if (texture.id != 0) {
            UnloadTexture(texture);
        }
    }
}


} // namespace match3
