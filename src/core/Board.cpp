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

std::size_t Board::index(const std::size_t x, const std::size_t y) const {
    return y * width_ + x;
}

Tile Board::randomTile() {
    std::uniform_int_distribution<int> distribution(0, static_cast<int>(Tile::Count) - 1);
    return static_cast<Tile>(distribution(rng_));
}

} // namespace match3
