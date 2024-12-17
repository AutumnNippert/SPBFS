#pragma once
#include "search.hpp"
#include "sliding_puzzle.hpp"
#include <cmath>

using namespace std;
using namespace SlidingPuzzle;

class SlidingPuzzleSolver {
public:
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
            float distance = 0;
            for (int i = 0; i < SIZE; i++) {
                for (int j = 0; j < SIZE; j++) {
                    int value = current.board[i][j];
                    if (value != EMPTY_TILE) {
                        int targetRow = (value - 1) / SIZE;
                        int targetCol = (value - 1) % SIZE;
                        distance += abs(i - targetRow) + abs(j - targetCol);
                    }
                }
            }
            return distance;
        };

        auto getCost = [](const State&, const State&) {
            return 1.0f;  // Each move costs 1
        };

        auto hash = [](const State& state) -> size_t {
            return std::hash<string>{}(state.toString());
        };
        
        return searcher.findPath(
            initial,
            isGoal,
            getSuccessors,
            manhattanDistance,
            getCost,
            hash
        );
    }
};