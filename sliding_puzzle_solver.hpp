#pragma once
#include "search.hpp"
#include "sliding_puzzle.hpp"
#include <cmath>

using namespace std;
using namespace SlidingPuzzle;

class SlidingPuzzleSolver {
public:

    static pair<State, State> parseInput(std::istream& input) {
        State state;
        state.board.resize(SIZE, std::vector<int>(SIZE));
        
        // Read dimensions
        int rows, cols;
        input >> rows >> cols;

        std::cout << "Rows: " << rows << ", Cols: " << cols << std::endl;

        // Skip the "starting positions for each tile:" line
        std::string line;
        std::getline(input, line); // Read the line "starting positions for each tile:"

        // Read starting positions for each tile
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                std::getline(input, line);
                input >> state.board[i][j];
                if (state.board[i][j] == EMPTY_TILE) {
                    state.empty = {i, j};  // Set the position of the empty tile
                }
            }
        }

        // Skip the "goal positions:" line
        std::getline(input, line); 
        State goal;
        goal.board.resize(SIZE, std::vector<int>(SIZE));
        
        // Read goal positions
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                std::getline(input, line);
                input >> goal.board[i][j];
                if (goal.board[i][j] == EMPTY_TILE) {
                    goal.empty = {i, j};  // Set the position of the empty tile in the goal state
                }
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
            int distance = 0;
            for (int i = 0; i < SIZE; i++) {
                for (int j = 0; j < SIZE; j++) {
                    int value = current.board[i][j];
                    if (value != EMPTY_TILE) {
                        int goalRow = (value - 1) / SIZE;
                        int goalCol = (value - 1) % SIZE;
                        distance += abs(i - goalRow) + abs(j - goalCol);
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