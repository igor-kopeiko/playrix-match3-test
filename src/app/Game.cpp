#include "app/Game.h"

#include "raylib.h"

#include <array>

namespace match3 {
namespace {

constexpr int kCellSize = 64;
constexpr int kBoardOffsetX = 44;
constexpr int kBoardOffsetY = 76;

Color colorForTile(const Tile tile) {
    constexpr std::array<Color, static_cast<std::size_t>(Tile::Count)> colors{
        Color{230, 70, 70, 255},
        Color{80, 190, 100, 255},
        Color{70, 120, 235, 255},
        Color{245, 205, 65, 255},
        Color{160, 90, 220, 255},
        Color{245, 145, 55, 255},
    };
    return colors.at(static_cast<std::size_t>(tile));
}

} // namespace

Game::Game() = default;

void Game::tick() {
    update();
    draw();
}

void Game::update() {
    // Input and animation state will live here.
}

void Game::draw() const {
    BeginDrawing();
    ClearBackground(Color{24, 27, 36, 255});

    DrawText("Match-3 C++ / WebAssembly", kBoardOffsetX, 28, 24, RAYWHITE);

    for (std::size_t y = 0; y < board_.height(); ++y) {
        for (std::size_t x = 0; x < board_.width(); ++x) {
            const int px = kBoardOffsetX + static_cast<int>(x) * kCellSize;
            const int py = kBoardOffsetY + static_cast<int>(y) * kCellSize;
            const Rectangle cell{
                static_cast<float>(px + 4),
                static_cast<float>(py + 4),
                static_cast<float>(kCellSize - 8),
                static_cast<float>(kCellSize - 8),
            };

            DrawRectangleRounded(cell, 0.28F, 8, colorForTile(board_.at(x, y)));
        }
    }

    EndDrawing();
}

} // namespace match3
