#pragma once

#include "core/Tile.h"
#include "core/Position.h"

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>
#include <unordered_set>

namespace match3 {

class Board {
public:
    Board(std::size_t width, std::size_t height, std::uint32_t seed = std::random_device{}());

    [[nodiscard]] std::size_t width() const noexcept { return width_; }
    [[nodiscard]] std::size_t height() const noexcept { return height_; }
    [[nodiscard]] Tile at(std::size_t x, std::size_t y) const;
    [[nodiscard]] Tile at(Position pos) const;
    void set(std::size_t x, std::size_t y, Tile tile);
    void set(Position pos, Tile tile);

    bool contains(std::size_t x, std::size_t y) const;
    bool contains(Position pos) const;

    bool are_neighbours(Position first, Position second) const noexcept;
    bool try_swap(Position first, Position second);

    std::vector<Position> find_matches() const;
    bool has_possible_moves();

    bool try_match_swap(Position first, Position second);
    
    void delete_cells(std::vector<Position>& cells_to_delete);
    

private:
    void create_board();
    bool has_matches_in_area_of(Position first, Position second) const;
    std::vector<Position> breadth_first_search(Position start, Tile wish_tile) const; //поиск в ширину
    void bf_search_check_cell(
        std::unordered_set<Position, PositionHash>& already_checked,
        std::vector<Position>& need_to_check,
        std::vector<Position>& result,
        Position& pos_to_check,
        Tile& wish_tile
    ) const;

    [[nodiscard]] std::size_t index(std::size_t x, std::size_t y) const;
    [[nodiscard]] Tile randomTile();

    std::size_t width_{};
    std::size_t height_{};
    std::vector<Tile> tiles_;
    std::mt19937 rng_;
};

} // namespace match3
