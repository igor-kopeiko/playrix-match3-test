#pragma once

#include "core/Tile.h"

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace match3 {

class Board {
public:
    Board(std::size_t width, std::size_t height, std::uint32_t seed = std::random_device{}());

    [[nodiscard]] std::size_t width() const noexcept { return width_; }
    [[nodiscard]] std::size_t height() const noexcept { return height_; }
    [[nodiscard]] Tile at(std::size_t x, std::size_t y) const;

private:
    [[nodiscard]] std::size_t index(std::size_t x, std::size_t y) const;
    [[nodiscard]] Tile randomTile();

    std::size_t width_{};
    std::size_t height_{};
    std::vector<Tile> tiles_;
    std::mt19937 rng_;
};

} // namespace match3
