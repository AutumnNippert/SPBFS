#pragma once
#include <cstddef> // for size_t
#include "boost/functional/hash.hpp"

struct Position {
    size_t row, col;

    inline bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};

// Define a free function 'hash_value' for Boost
inline std::size_t hash_value(const Position& pos) {
    std::size_t seed = 0;
    boost::hash_combine(seed, pos.row);
    boost::hash_combine(seed, pos.col);
    return seed;
}

namespace std {
    template <>
    struct hash<Position> {
        std::size_t operator()(const Position& pos) const noexcept {
            return hash_value(pos);
        }
    };
}