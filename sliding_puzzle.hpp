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
        int row, col;
        
        bool operator==(const Position& other) const {
            return row == other.row && col == other.col;
        }
    };

    struct State {
        vector<vector<int>> board;
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
            for (const auto& row : board) {
                for (int val : row) {
                    ss << val << ",";
                }
            }
            return ss.str();
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const State& state) {
        for (const auto& row : state.board) {
            for (int val : row) {
                os << val << " ";
            }
            os << "\n";
        }
        return os;
    }

    inline State createGoalState() {
        State goal;
        goal.board = vector<vector<int>>(SIZE, vector<int>(SIZE));
        int value = 1;
        
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                goal.board[i][j] = value++;
            }
        }
        goal.board[SIZE-1][SIZE-1] = EMPTY_TILE;
        goal.empty = {SIZE-1, SIZE-1};
        return goal;
    }

    inline vector<Position> getValidMoves(const State& state) {
        vector<Position> moves;
        const int dr[] = {-1, 1, 0, 0};  // up, down, left, right
        const int dc[] = {0, 0, -1, 1};
        
        for (int i = 0; i < 4; i++) {
            int newRow = state.empty.row + dr[i];
            int newCol = state.empty.col + dc[i];
            
            if (newRow >= 0 && newRow < SIZE && newCol >= 0 && newCol < SIZE) {
                moves.push_back({newRow, newCol});
            }
        }
        return moves;
    }

    inline void applyMove(State& state, Position move) {
        swap(state.board[state.empty.row][state.empty.col],
             state.board[move.row][move.col]);
        state.empty = move;
    }
} 