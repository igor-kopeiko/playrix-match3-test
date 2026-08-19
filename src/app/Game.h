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
    Filling     // появляются новые фишки
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
    float swapping = 1.0; //back_swapping выполняется то же время
    float removing = 1.0;
    //float back_swapping = 1.0;

};

//struct LevelConfig {
//    std::size_t width;
//    std::size_t height;
//};

class Game {
public:
    Game(int kBoardOffsetX, int kBoardOffsetY, int kCellSize, int space_near_cell, std::size_t cell_amount_x, std::size_t cell_amount_y);
    void tick();

private:
    void update();

    void update_idle();
    void update_swapping();
    void update_back_swapping();
    void update_removing();
    void update_falling();
    void update_filling();




    void draw() const;
    void draw_regular_cell(std::size_t x, std::size_t y) const;
    void draw_swapping_cell(std::size_t x, std::size_t y) const;
    void draw_removing_cell(std::size_t x, std::size_t y) const;

    const int kBoardOffsetX; //отступ первой ячейки от края
    const int kBoardOffsetY;

    const int kCellSize;
    const int space_near_cell; //то же самое что расстояние между клетками/2

    const std::size_t cell_amount_x;
    const std::size_t cell_amount_y;

    //вернет индексы выбранной клетки
    std::optional<Position> get_chosen_cell(Vector2 mouse);

    //определяет направление движения swapping
    void calculate_swapping_shift();
    

    //LevelConfig level_{8, 8}; //пока оставим дефолт
    Board board_{ cell_amount_x, cell_amount_y, 42 };
    
    std::optional<Position> first_selected_cell = std::nullopt;

    GameState current_game_state = GameState::Idle;

    Swapping_poses swapping_poses{ {},{} };

    float animation_timer = 0.0;

    Swapping_shift swapping_shift{};

    Animation_duration animation_duration{};

    std::vector<Position> cells_to_remove;

};

} // namespace match3

