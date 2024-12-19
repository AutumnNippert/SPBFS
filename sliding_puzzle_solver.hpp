#pragma once
#include "search.hpp"
#include "sliding_puzzle.hpp"
#include <cmath>
#include <iostream>

using namespace std;
using namespace SlidingPuzzle;

class SlidingPuzzleSolver {

public:

    static pair<State, State> parseInput(std::istream& input) {
        State state;
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

    static vector<State> solve(
        const State& initial,
        const State& goal,
        Search<State, float>& searcher
    ) {

        auto isGoal = [&goal](const State& state) {
            return state == goal;
        };

        auto getSuccessors = [](const State& state) {
            vector<State> successors;
            for (const auto& move : getValidMoves(state)) {
                State newState = state;
                applyMove(newState, move);
                successors.push_back(newState);
            }
            return successors;
        };

        auto manhattanDistance = [](const State& current, const State& goal) {
            // for each tile in the current state, calculate the manhattan distance to the goal state and sum them up

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
                int goalRow = goalIndex / SIZE;
                int goalCol = goalIndex % SIZE;
                int currentRow = i / SIZE;
                int currentCol = i % SIZE;
                distance += abs(goalRow - currentRow) + abs(goalCol - currentCol);
            }
            return distance - 1;
        };

        auto returnZero = [](const State&, const State&) {
            return 0.0f;
        };

        auto getCost = [](const State&, const State&) {
            return 1.0f;  // Each move costs 1
        };

        auto hash = [](const State& state) -> size_t {
            size_t h = state.board[0];
            for (int i = 1; i < SlidingPuzzle::SIZE * SlidingPuzzle::SIZE; i++)
                h += h * 3 + state.board[i];
            return h;
        };
        
        searcher.initialize(initial, goal, getSuccessors, manhattanDistance, getCost, hash);
        return searcher.findPath();
    }
};