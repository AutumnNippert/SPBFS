#include <cstddef> // for size_t

struct Position {
    size_t row, col;

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};

struct PositionHash {
    size_t operator()(const Position& pos) const {
        return pos.row * 1000 + pos.col; // Assuming the grid is at most 1000x1000 because the struct has no information about the grid size
    }
};