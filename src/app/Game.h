#pragma once

#include "raylib.h"
#include "core/Board.h"
#include "core/Position.h"

#include <iostream>
#include <optional>
#include <vector>

namespace match3 {

enum class GameState {
    Idle,       // ждём действий игрока
    Swapping,   // смена
    BackSwapping,//если swap недопустим
    Removing,   // удаляем совпавшие фишки
    Falling,    // фишки падают вниз
    Checking,     //проверка наличие match
    Shuffling //перемешивание в случае отсуствия возможных решений
};

struct Swapping_shift {
    int x;
    int y;
};

struct Swapping_poses {
    Position first;
    Position second;
};

struct Animation_duration {
    float swapping; //back_swapping выполняется то же время
    float removing;
    float fall_speed_pix_pro_sec;

};



struct LevelGoal {
    Tile color;
    int amount;
};

struct LevelConfig {
    int id;
    std::string name;

    int width;
    int height;
    int colors_count;
    int moves;

    std::vector<LevelGoal> goals;
};

class Game {
public:
    Game(LevelConfig level_config, int screenWidth, int screenHeight);
    void tick();

private:
    void update();

    void update_idle();
    void update_swapping();
    void update_back_swapping();
    void update_removing();
    void update_falling();
    void update_checking();
    void update_shuffling();




    void draw() const;
    void draw_regular_cell(std::size_t x, std::size_t y) const;
    void draw_swapping_cell(std::size_t x, std::size_t y) const;
    void draw_removing_cell(std::size_t x, std::size_t y) const;
    void draw_fallen_cell(FallMove fall_cell) const;

    LevelConfig level_config;

    const int screenWidth;
    const int screenHeight;



    int kBoardOffsetX; //отступ первой ячейки от края
    int kBoardOffsetY;

    int kCellSize_x;
    int kCellSize_y;
    int space_near_cell; //то же самое что расстояние между клетками/2

    //std::size_t cell_amount_x;
    //std::size_t cell_amount_y;

    //вернет индексы выбранной клетки
    std::optional<Position> get_chosen_cell(Vector2 mouse);

    //определяет направление движения swapping
    void calculate_swapping_shift();
    

    //LevelConfig level_{8, 8}; //пока оставим дефолт
    Board board_;//{ cell_amount_x, cell_amount_y, 42 };
    
    std::optional<Position> first_selected_cell = std::nullopt;

    GameState current_game_state = GameState::Idle;

    Swapping_poses swapping_poses{ {},{} };

    float animation_timer = 0.0;

    Swapping_shift swapping_shift{};

    Animation_duration animation_duration{};

    std::vector<Position> cells_to_remove;

    std::vector<FallMove> falling_cells;
    float max_falling_time = 0.0;




};

} // namespace match3

