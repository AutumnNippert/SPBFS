#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>

using namespace std;

namespace SlidingPuzzle {
    static constexpr int SIZE = 4;  // 4x4 puzzle
    static constexpr int EMPTY_TILE = 0;
    
    struct Position {
        int index;
        
        bool operator==(const Position& other) const {
            return index == other.index;
        }
    };

    struct State {
        vector<int> board;
        Position empty;

        bool operator==(const State& other) const {
            return board == other.board;
        }

        // Required for use as key in unordered_map
        bool operator<(const State& other) const {
            return toString() < other.toString();
        }

        string toString() const {
            stringstream ss;
            for (int val : board) {
                ss << val << ",";
            }
            return ss.str();
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const State& state) {
        for (int val : state.board) {
            os << val << " ";
        }
        os << "\n";
        return os;
    }

    inline State createGoalState() {
        State goal;
        goal.board = vector<int>(SIZE * SIZE);
        int value = 1;
        
        for (int i = 0; i < SIZE * SIZE; i++) {
            goal.board[i] = value++;
        }
        goal.board[SIZE * SIZE - 1] = EMPTY_TILE;
        goal.empty = {SIZE * SIZE - 1};
        return goal;
    }

    inline vector<Position> getValidMoves(const State& state) {
        vector<Position> moves;
        if (state.empty.index % SIZE != 0) {
            moves.push_back({state.empty.index - 1});
        }
        if (state.empty.index % SIZE != SIZE - 1) {
            moves.push_back({state.empty.index + 1});
        }
        if (state.empty.index >= SIZE) {
            moves.push_back({state.empty.index - SIZE});
        }
        if (state.empty.index < SIZE * (SIZE - 1)) {
            moves.push_back({state.empty.index + SIZE});
        }
        return moves;
    }

    inline void applyMove(State& state, Position move) {
        swap(state.board[state.empty.index], state.board[move.index]);
        state.empty = move;
    }
} 