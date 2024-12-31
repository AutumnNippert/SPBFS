#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>

#include "search.hpp"
#include <cmath>

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

        bool operator == (const State& other) const {
            return board == other.board;
        }

        // Required for use as key in unordered_map
        bool operator < (const State& other) const {
            return this->hash() < other.hash();
        }

        string toString() const {
            stringstream ss;
            for (int val : board) {
                ss << val << ",";
            }
            return ss.str();
        }

        size_t hash() const {
            size_t h = board[0];
            for (int i = 1; i < SIZE * SIZE; i++)
                h += h * 3 + board[i];
            return h;
        }
    };

    inline std::ostream& operator << (std::ostream& os, const State& state) {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                fprintf(stdout, "%2d ", state.board[i * SIZE + j]);
            }
            fprintf(stdout, "\n");
        }
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
        if (state.empty.index >= SIZE) {
            moves.push_back({state.empty.index - SIZE});
        }
        if (state.empty.index % SIZE > 0) {
            moves.push_back({state.empty.index - 1});
        }
        if (state.empty.index % SIZE < SIZE - 1) {
            moves.push_back({state.empty.index + 1});
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

    class SlidingPuzzleSolver {
    public:

        static pair<State, State> parseInput(std::istream& input) {
            State state = {};
            state.board.resize(SIZE * SIZE); // Adjusted to 1D array
            
            // Read dimensions
            int rows, cols;
            input >> rows >> cols;

            std::cout << "Rows: " << rows << ", Cols: " << cols << std::endl;

            // Skip the "starting positions for each tile:" line
            std::string line;
            std::getline(input, line); // Read the line "starting positions for each tile:"

            // Read starting positions for each tile
            for (int i = 0; i < SIZE * SIZE; i++) {
                std::getline(input, line); // Read the line "starting positions for each tile:"
                input >> state.board[i];
                if (state.board[i] == EMPTY_TILE) {
                    state.empty.index = i;  // Set the position of the empty tile
                }
            }

            // Skip the "goal positions:" line
            std::getline(input, line); 
            State goal;
            goal.board.resize(SIZE * SIZE); // Adjusted to 1D array
            
            // Read goal positions
            for (int i = 0; i < SIZE * SIZE; i++) {
                std::getline(input, line); // Read the line "starting positions for each tile:"
                input >> goal.board[i];
                if (goal.board[i] == EMPTY_TILE) {
                    goal.empty.index = i;  // Set the position of the empty tile in the goal state
                }
            }

            return {state, goal};
        }

        static bool isGoal(const State& state, const State& goal) {
            return state == goal;
        }

        static vector<State> getSuccessors(const State& state) {
            vector<State> successors;
            for (const auto& move : getValidMoves(state)) {
                State newState = state;
                applyMove(newState, move);
                successors.push_back(newState);
            }
            return successors;
        }

        static int manhattanDistance(const State& current, const State& goal) {
            int goalIndexLookup[SIZE * SIZE];
            for (int i = 0; i < SIZE * SIZE; i++) {
                goalIndexLookup[goal.board[i]] = i;
            }

            int distance = 0;
            for (int i = 0; i < SIZE * SIZE; i++) {
                int currentTile = current.board[i];
                if (currentTile == EMPTY_TILE) {
                    continue;
                }
                int goalIndex = goalIndexLookup[currentTile];
                distance += abs(goalIndex / SIZE - i / SIZE) + abs(goalIndex % SIZE - i % SIZE);
            }
            return distance;
        }

        static float returnZero(const State&, const State&) {
            return 0.0f;
        }

        static float getCost(const State&, const State&) {
            return 1.0f;  // Each move costs 1
        }

        static size_t hash(const State& state) {
            size_t h = state.board[0];
            for (int i = 1; i < SlidingPuzzle::SIZE * SlidingPuzzle::SIZE; i++)
                h += h * 3 + state.board[i];
            return h;
        }

        // static vector<State> solve(
        //     const State& initial,
        //     const State& goal,
        //     Search<State, float>& searcher
        // ) {
        //     SlidingPuzzleSolver solver;
        //     searcher.initialize(initial, goal, getSuccessors, manhattanDistance, getCost, hash);
        //     return searcher.findPath();
        // }
    };
}