#include "core/Board.h"

#include <stdexcept>

namespace match3 {

Board::Board(const std::size_t width, const std::size_t height, const std::uint32_t seed)
    : width_(width), height_(height), tiles_(width * height), rng_(seed) {
    if (width == 0 || height == 0) {
        throw std::invalid_argument("Board dimensions must be greater than zero");
    }

    for (auto& tile : tiles_) {
        tile = randomTile();
    }
}

Tile Board::at(const std::size_t x, const std::size_t y) const {
    if (x >= width_ || y >= height_) {
        throw std::out_of_range("Board coordinates are out of range");
    }
    return tiles_[index(x, y)];
}

Tile Board::at(Position pos) const {
    return at(pos.x, pos.y);
}

std::size_t Board::index(const std::size_t x, const std::size_t y) const {
    return y * width_ + x;
}

Tile Board::randomTile() {
    std::uniform_int_distribution<int> distribution(0, static_cast<int>(Tile::Count) - 1);
    return static_cast<Tile>(distribution(rng_));
}

//===============================

void Board::set(std::size_t x, std::size_t y, Tile tile) {
    //выставляем цвет по клетке
    if (x >= width_ || y >= height_) {
        throw std::out_of_range("Board coordinates are out of range");
    }
    tiles_[index(x, y)] = tile;
}

void Board::set(Position pos, Tile tile) {
    //выставляем цвет по клетке
    set(pos.x, pos.y, tile);
}

bool Board::contains(std::size_t x, std::size_t y) const {
    //проверяем принадлежность
    if (x >= width_ || y >= height_) {
        return false;
    }
    return true;
}

bool Board::contains(Position pos) const {
    return contains(pos.x, pos.y);
}





bool Board::are_neighbours(Position first, Position second) const noexcept {
    //проверяем что это соседние клетки
    if (!contains(first)) {
        return false;
    }
    if (!contains(second)) {
        return false;
    }

    //по горизонтали
    if (first.x == second.x + 1 || first.x + 1 == second.x) {
        if (first.y == second.y) {
            return true;
        }
    }

    //по вертикали
    if (first.y == second.y + 1 || first.y + 1 == second.y) {
        if (first.x == second.x) {
            return true;
        }
    }

    return false;
}

bool Board::try_swap(Position first, Position second) {


    if (!are_neighbours(first, second)) {
        return false;
    }
    else {
        Tile temp = at(first);
        set(first, at(second));
        set(second, temp);
        return true;
    }
}












} // namespace match3
