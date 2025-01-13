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

    typedef size_t position;

    struct State {
        vector<int> board;
        position empty; // position of the empty tile

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

    inline void applyMove(State& state, position move) {
        swap(state.board[state.empty], state.board[move]);
        state.empty = move;
    }

    template<typename State, typename Cost = float>
    class SlidingTileInstance: public ProblemInstance<State, Cost> {
    public:
        SlidingTileInstance(const State& initial, const State& goal) : ProblemInstance<State, Cost>(initial), goal(goal) {
            this->goal = goal;
        }

        static SlidingTileInstance parseInput(std::istream& input) {
            State state, goal;
            std::string line; // for inputs

            state.board.resize(SIZE * SIZE); 
            goal.board.resize(SIZE * SIZE);

            // Read dimensions
            int rows, cols;
            input >> rows >> cols;
            
            std::getline(input, line); // Skip the "starting positions for each tile:" line

            // Read starting positions for each tile
            for (size_t i = 0; i < SIZE * SIZE; i++) {
                std::getline(input, line); 
                input >> state.board[i];
                if (state.board[i] == EMPTY_TILE) {
                    state.empty = i;  // Set the position of the empty tile
                }
            }

            std::getline(input, line); // Skip the "goal positions:" line
            
            // Read goal positions
            for (size_t i = 0; i < SIZE * SIZE; i++) {
                std::getline(input, line); // Read the line "starting positions for each tile:"
                input >> goal.board[i];
                if (goal.board[i] == EMPTY_TILE) {
                    goal.empty = i;  // Set the position of the empty tile in the goal state
                }
            }
            return SlidingTileInstance(state, goal);
        }

        inline vector<position> getValidMoves(const State& state) const {
            vector<position> moves;
            if (state.empty >= SIZE) {
                moves.push_back({state.empty - SIZE});
            }
            if (state.empty % SIZE > 0) {
                moves.push_back({state.empty - 1});
            }
            if (state.empty % SIZE < SIZE - 1) {
                moves.push_back({state.empty + 1});
            }
            if (state.empty < SIZE * (SIZE - 1)) {
                moves.push_back({state.empty + SIZE});
            }
            return moves;
        }
        
        // The functions required by ProblemInstance

        float heuristic(const State& state) const override {
            int goalIndexLookup[SIZE * SIZE];
            for (int i = 0; i <= SIZE * SIZE; i++) {
                goalIndexLookup[goal.board[i]] = i;
            }

            int distance = 0;
            for (int i = 0; i < SIZE * SIZE; i++) {
                int currentTile = state.board[i];
                if (currentTile == EMPTY_TILE) {
                    continue;
                }
                int goalIndex = goalIndexLookup[currentTile];
                distance += abs(goalIndex / SIZE - i / SIZE) + abs(goalIndex % SIZE - i % SIZE);
            }
            return distance;
        }

        vector<State> getSuccessors(const State& state) const override {
            vector<State> successors;
            for (const auto& move : getValidMoves(state)) {
                State newState = state;
                applyMove(newState, move);
                successors.push_back(newState);
            }
            return successors;
        }

        float getCost(const State&, const State&) const override {
            return 1.0f;  // Each move costs 1
        }

        size_t hash(const State& state) const override {
            size_t h = state.board[0];
            for (int i = 1; i < SlidingPuzzle::SIZE * SlidingPuzzle::SIZE; i++)
                h += h * 3 + state.board[i];
            return h;
        }

        inline size_t maxActionCount() const override {
            return 4;
        }

    private:
        State goal;
    };
}