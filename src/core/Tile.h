#pragma once

#include <cstdint>

namespace match3 {

enum class Tile : std::uint8_t {
    Default,
    Red,
    Green,
    Blue,
    Yellow,
    Purple,
    Orange,
    Count
};

} // namespace match3
