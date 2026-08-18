#pragma once

#include<cstddef>
#include <functional> // для std::hash

struct Position {
    std::size_t x;
    std::size_t y;
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct PositionHash {
    std::size_t operator()(const Position& pos) const noexcept {
        const std::size_t hashX = std::hash<std::size_t>{}(pos.x);
        const std::size_t hashY = std::hash<std::size_t>{}(pos.y);

        return hashX ^ (hashY << 1);
    }
};
