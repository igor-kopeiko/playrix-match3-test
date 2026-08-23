#include "core/Board.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <optional>
#include <vector>

namespace {

using match3::Board;
using match3::FallMove;
using match3::Tile;

constexpr int kColorAmount = 6;
constexpr std::uint32_t kSeed = 12345;

void fill_without_matches(Board& board) {
    constexpr Tile pattern[] = {
        Tile::Red,
        Tile::Green,
        Tile::Blue,
        Tile::Yellow,
        Tile::Purple,
    };

    for (std::size_t y = 0; y < board.height(); ++y) {
        for (std::size_t x = 0; x < board.width(); ++x) {
            board.set(x, y, pattern[(x + y) % std::size(pattern)]);
        }
    }
}

bool contains_position(const std::vector<Position>& positions, Position expected) {
    return std::find(positions.begin(), positions.end(), expected) != positions.end();
}

bool contains_fall_move(
    const std::vector<FallMove>& moves,
    Tile tile,
    Position from,
    Position to
) {
    return std::any_of(moves.begin(), moves.end(), [&](const FallMove& move) {
        return move.tile == tile && move.from == from && move.to == to;
    });
}

} // namespace

TEST_CASE("Board keeps requested dimensions") {
    const Board board{8, 10, kColorAmount, kSeed};

    CHECK(board.width() == 8);
    CHECK(board.height() == 10);
}

TEST_CASE("Board rejects zero-sized dimensions") {
    CHECK_THROWS_AS(Board(0, 8, kColorAmount, kSeed), std::invalid_argument);
    CHECK_THROWS_AS(Board(8, 0, kColorAmount, kSeed), std::invalid_argument);
}

TEST_CASE("Board rejects coordinates outside the board") {
    const Board board{8, 8, kColorAmount, kSeed};

    CHECK_NOTHROW(board.at(7, 7));
    CHECK_THROWS_AS(board.at(8, 7), std::out_of_range);
    CHECK_THROWS_AS(board.at(7, 8), std::out_of_range);
}

TEST_CASE("set and at store tiles at the requested positions") {
    Board board{4, 4, kColorAmount, kSeed};

    board.set(1, 2, Tile::Purple);
    board.set(Position{3, 0}, Tile::Orange);

    CHECK(board.at(1, 2) == Tile::Purple);
    CHECK(board.at(Position{3, 0}) == Tile::Orange);
    CHECK_THROWS_AS(board.set(4, 0, Tile::Red), std::out_of_range);
}

TEST_CASE("contains reports whether coordinates belong to the board") {
    const Board board{4, 3, kColorAmount, kSeed};

    CHECK(board.contains(0, 0));
    CHECK(board.contains(3, 2));
    CHECK_FALSE(board.contains(4, 2));
    CHECK_FALSE(board.contains(3, 3));
    CHECK_FALSE(board.contains(Position{-1, 0}));
    CHECK_FALSE(board.contains(Position{0, -1}));
}

TEST_CASE("Randomly generated boards start without automatic matches") {
    for (std::uint32_t seed = 1; seed <= 20; ++seed) {
        const Board board{10, 10, kColorAmount, seed};
        CHECK(board.find_matches().empty());
    }
}

TEST_CASE("Randomly generated tiles stay inside configured color range") {
    const Board board{12, 12, kColorAmount, kSeed};

    for (std::size_t y = 0; y < board.height(); ++y) {
        for (std::size_t x = 0; x < board.width(); ++x) {
            const auto value = static_cast<int>(board.at(x, y));
            CHECK(value >= static_cast<int>(Tile::Red));
            CHECK(value <= kColorAmount);
        }
    }
}

TEST_CASE("Initial map keeps fixed tiles") {
    const std::vector<std::vector<std::optional<Tile>>> map{
        {Tile::Red, Tile::Green, Tile::Blue},
        {Tile::Green, Tile::Blue, Tile::Red},
        {Tile::Blue, Tile::Red, Tile::Green},
    };

    const Board board{3, 3, kColorAmount, map, kSeed};

    for (std::size_t y = 0; y < map.size(); ++y) {
        for (std::size_t x = 0; x < map[y].size(); ++x) {
            REQUIRE(map[y][x].has_value());
            CHECK(board.at(x, y) == map[y][x].value());
        }
    }
}

TEST_CASE("Initial map rejects incorrect dimensions") {
    const std::vector<std::vector<std::optional<Tile>>> wrong_height{
        {Tile::Red, Tile::Green, Tile::Blue},
        {Tile::Green, Tile::Blue, Tile::Red},
    };

    const std::vector<std::vector<std::optional<Tile>>> wrong_width{
        {Tile::Red, Tile::Green},
        {Tile::Green, Tile::Blue},
        {Tile::Blue, Tile::Red},
    };

    CHECK_THROWS_AS(Board(3, 3, kColorAmount, wrong_height, kSeed), std::invalid_argument);
    CHECK_THROWS_AS(Board(3, 3, kColorAmount, wrong_width, kSeed), std::invalid_argument);
}

TEST_CASE("Initial map rejects a pre-existing match") {
    const std::vector<std::vector<std::optional<Tile>>> map{
        {Tile::Red, Tile::Red, Tile::Red},
        {Tile::Green, Tile::Blue, Tile::Yellow},
        {Tile::Blue, Tile::Yellow, Tile::Green},
    };

    CHECK_THROWS_AS(Board(3, 3, kColorAmount, map, kSeed), std::runtime_error);
}

TEST_CASE("are_neighbours accepts only orthogonally adjacent cells") {
    const Board board{4, 4, kColorAmount, kSeed};

    CHECK(board.are_neighbours({1, 1}, {2, 1}));
    CHECK(board.are_neighbours({1, 1}, {1, 2}));
    CHECK_FALSE(board.are_neighbours({1, 1}, {2, 2}));
    CHECK_FALSE(board.are_neighbours({1, 1}, {1, 1}));
    CHECK_FALSE(board.are_neighbours({0, 0}, {-1, 0}));
    CHECK_FALSE(board.are_neighbours({3, 3}, {4, 3}));
}

TEST_CASE("try_swap swaps neighbouring cells") {
    Board board{4, 4, kColorAmount, kSeed};
    fill_without_matches(board);

    const Tile first = board.at(0, 0);
    const Tile second = board.at(1, 0);

    REQUIRE(board.try_swap({0, 0}, {1, 0}));
    CHECK(board.at(0, 0) == second);
    CHECK(board.at(1, 0) == first);
}

TEST_CASE("try_swap rejects non-neighbouring cells without changing them") {
    Board board{4, 4, kColorAmount, kSeed};
    fill_without_matches(board);

    const Tile first = board.at(0, 0);
    const Tile second = board.at(2, 0);

    CHECK_FALSE(board.try_swap({0, 0}, {2, 0}));
    CHECK(board.at(0, 0) == first);
    CHECK(board.at(2, 0) == second);
}

TEST_CASE("find_matches finds exactly three horizontal tiles") {
    Board board{5, 5, kColorAmount, kSeed};
    fill_without_matches(board);

    board.set(0, 2, Tile::Orange);
    board.set(1, 2, Tile::Orange);
    board.set(2, 2, Tile::Orange);

    const auto matches = board.find_matches();

    REQUIRE(matches.size() == 3);
    CHECK(contains_position(matches, {0, 2}));
    CHECK(contains_position(matches, {1, 2}));
    CHECK(contains_position(matches, {2, 2}));
}

TEST_CASE("find_matches finds exactly three vertical tiles") {
    Board board{5, 5, kColorAmount, kSeed};
    fill_without_matches(board);

    board.set(3, 0, Tile::Orange);
    board.set(3, 1, Tile::Orange);
    board.set(3, 2, Tile::Orange);

    const auto matches = board.find_matches();

    REQUIRE(matches.size() == 3);
    CHECK(contains_position(matches, {3, 0}));
    CHECK(contains_position(matches, {3, 1}));
    CHECK(contains_position(matches, {3, 2}));
}

TEST_CASE("find_matches includes every tile in a line longer than three") {
    Board board{6, 6, kColorAmount, kSeed};
    fill_without_matches(board);

    for (int x = 1; x <= 4; ++x) {
        board.set(x, 3, Tile::Orange);
    }

    const auto matches = board.find_matches();

    REQUIRE(matches.size() == 4);
    for (int x = 1; x <= 4; ++x) {
        CHECK(contains_position(matches, {x, 3}));
    }
}

TEST_CASE("find_matches does not duplicate cells at an intersection") {
    Board board{5, 5, kColorAmount, kSeed};
    fill_without_matches(board);

    for (int x = 1; x <= 3; ++x) {
        board.set(x, 2, Tile::Orange);
    }
    for (int y = 1; y <= 3; ++y) {
        board.set(2, y, Tile::Orange);
    }

    const auto matches = board.find_matches();

    CHECK(matches.size() == 5);
    CHECK(contains_position(matches, {2, 2}));
}

TEST_CASE("try_match_swap keeps a swap that creates a match") {
    Board board{4, 4, kColorAmount, kSeed};
    fill_without_matches(board);

    board.set(0, 2, Tile::Red);
    board.set(1, 2, Tile::Red);
    board.set(2, 2, Tile::Green);
    board.set(3, 2, Tile::Red);

    REQUIRE(board.find_matches().empty());
    REQUIRE(board.try_match_swap({2, 2}, {3, 2}));

    CHECK(board.at(0, 2) == Tile::Red);
    CHECK(board.at(1, 2) == Tile::Red);
    CHECK(board.at(2, 2) == Tile::Red);
    CHECK_FALSE(board.find_matches().empty());
}

TEST_CASE("try_match_swap restores a swap that does not create a match") {
    Board board{5, 5, kColorAmount, kSeed};
    fill_without_matches(board);

    const Tile first = board.at(0, 0);
    const Tile second = board.at(1, 0);

    REQUIRE(board.find_matches().empty());
    CHECK_FALSE(board.try_match_swap({0, 0}, {1, 0}));
    CHECK(board.at(0, 0) == first);
    CHECK(board.at(1, 0) == second);
}

TEST_CASE("delete_cells clears only requested positions") {
    Board board{4, 4, kColorAmount, kSeed};
    fill_without_matches(board);

    const Tile untouched = board.at(3, 3);
    std::vector<Position> to_delete{{0, 0}, {1, 1}, {2, 2}};

    board.delete_cells(to_delete);

    CHECK(board.at(0, 0) == Tile::Default);
    CHECK(board.at(1, 1) == Tile::Default);
    CHECK(board.at(2, 2) == Tile::Default);
    CHECK(board.at(3, 3) == untouched);
}

TEST_CASE("collapse_cells moves tiles downward into empty cells") {
    Board board{3, 5, kColorAmount, kSeed};
    fill_without_matches(board);

    board.set(0, 4, Tile::Default);
    board.set(0, 2, Tile::Default);

    const Tile old_y3 = board.at(0, 3);
    const Tile old_y1 = board.at(0, 1);
    const Tile old_y0 = board.at(0, 0);

    const auto moves = board.collapse_cells();

    CHECK(board.at(0, 4) == old_y3);
    CHECK(board.at(0, 3) == old_y1);
    CHECK(board.at(0, 2) == old_y0);
    CHECK(board.at(0, 1) == Tile::Default);
    CHECK(board.at(0, 0) == Tile::Default);

    CHECK(contains_fall_move(moves, old_y3, {0, 3}, {0, 4}));
    CHECK(contains_fall_move(moves, old_y1, {0, 1}, {0, 3}));
    CHECK(contains_fall_move(moves, old_y0, {0, 0}, {0, 2}));

    CHECK(std::none_of(moves.begin(), moves.end(), [](const FallMove& move) {
        return move.tile == Tile::Default;
    }));
}

TEST_CASE("collapse_cells leaves a full board unchanged") {
    Board board{5, 5, kColorAmount, kSeed};
    fill_without_matches(board);

    const auto moves = board.collapse_cells();

    CHECK(moves.empty());
}

TEST_CASE("fill_empty_cells fills all empty positions with valid colors") {
    Board board{4, 5, kColorAmount, kSeed};
    fill_without_matches(board);

    board.set(0, 0, Tile::Default);
    board.set(0, 1, Tile::Default);
    board.set(2, 0, Tile::Default);

    const auto new_cells = board.fill_empty_cells();

    REQUIRE(new_cells.size() == 3);
    for (std::size_t y = 0; y < board.height(); ++y) {
        for (std::size_t x = 0; x < board.width(); ++x) {
            CHECK(board.at(x, y) != Tile::Default);
        }
    }

    for (const auto& move : new_cells) {
        const auto value = static_cast<int>(move.tile);
        CHECK(value >= static_cast<int>(Tile::Red));
        CHECK(value <= kColorAmount);
        CHECK(move.from.x == move.to.x);
        CHECK(move.from.y < 0);
        CHECK(move.to.y >= 0);
    }
}

TEST_CASE("has_possible_moves detects a board with a valid match-producing swap") {
    Board board{4, 4, kColorAmount, kSeed};
    fill_without_matches(board);

    board.set(0, 2, Tile::Red);
    board.set(1, 2, Tile::Red);
    board.set(2, 2, Tile::Green);
    board.set(3, 2, Tile::Red);

    REQUIRE(board.find_matches().empty());
    CHECK(board.has_possible_moves());
}

TEST_CASE("has_possible_moves does not modify the board") {
    Board board{5, 5, kColorAmount, kSeed};
    fill_without_matches(board);

    std::vector<Tile> before;
    for (std::size_t y = 0; y < board.height(); ++y) {
        for (std::size_t x = 0; x < board.width(); ++x) {
            before.push_back(board.at(x, y));
        }
    }

    (void)board.has_possible_moves();

    std::size_t index = 0;
    for (std::size_t y = 0; y < board.height(); ++y) {
        for (std::size_t x = 0; x < board.width(); ++x) {
            CHECK(board.at(x, y) == before[index++]);
        }
    }
}
