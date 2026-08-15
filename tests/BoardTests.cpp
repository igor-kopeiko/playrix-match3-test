#include "core/Board.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Board keeps requested dimensions") {
    const match3::Board board{8, 10, 123};

    CHECK(board.width() == 8);
    CHECK(board.height() == 10);
}

TEST_CASE("Board rejects zero-sized dimensions") {
    CHECK_THROWS_AS(match3::Board(0, 8, 123), std::invalid_argument);
    CHECK_THROWS_AS(match3::Board(8, 0, 123), std::invalid_argument);
}

TEST_CASE("Board rejects coordinates outside the board") {
    const match3::Board board{8, 8, 123};

    CHECK_NOTHROW(board.at(7, 7));
    CHECK_THROWS_AS(board.at(8, 7), std::out_of_range);
    CHECK_THROWS_AS(board.at(7, 8), std::out_of_range);
}
