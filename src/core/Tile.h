#pragma once

#include <cstdint>

namespace match3 {

enum class Tile : std::uint8_t {
    Red,
    Green,
    Blue,
    Yellow,
    Purple,
    Orange,
    Count,
    Default
};

} // namespace match3
