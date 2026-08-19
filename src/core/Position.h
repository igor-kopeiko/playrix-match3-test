#pragma once

#include<cstddef>
#include <functional> // для std::hash

struct Position {
    int x;
    int y;
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct PositionHash {
    std::size_t operator()(const Position& pos) const noexcept {
        const std::size_t hashX = std::hash<int>{}(pos.x);
        const std::size_t hashY = std::hash<int>{}(pos.y);

        return hashX ^ (hashY << 1);
    }
};
