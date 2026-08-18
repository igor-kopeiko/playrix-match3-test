#pragma once

#include "raylib.h"
#include "core/Board.h"
#include "core/Position.h"

#include <iostream>
#include <optional>
#include <vector>

namespace match3 {

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
    void draw() const;

    const int kBoardOffsetX; //отступ первой ячейки от края
    const int kBoardOffsetY;

    const int kCellSize;
    const int space_near_cell; //то же самое что расстояние между клетками/2

    const std::size_t cell_amount_x;
    const std::size_t cell_amount_y;

    //вернет индексы выбранной клетки
    std::optional<Position> get_chosen_cell(Vector2 mouse);

    //LevelConfig level_{8, 8}; //пока оставим дефолт
    Board board_{ cell_amount_x, cell_amount_y, 42 };
    
    std::optional<Position> first_selected_cell = std::nullopt;

};

} // namespace match3

