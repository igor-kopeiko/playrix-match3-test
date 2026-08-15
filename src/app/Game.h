#pragma once

#include "core/Board.h"

namespace match3 {

class Game {
public:
    Game();
    void tick();

private:
    void update();
    void draw() const;

    Board board_{8, 8, 42};
};

} // namespace match3
