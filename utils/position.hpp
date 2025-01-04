#pragma once
#include <cstddef> // for size_t

struct Position {
    size_t row, col;

    inline bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};

struct PositionHash {
    inline std::size_t operator()(const Position& pos) const {
        return pos.row * 1000 + pos.col;
    }
};