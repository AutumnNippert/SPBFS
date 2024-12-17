#include "sliding_puzzle_solver.hpp"
#include "astar.hpp"
#include <iostream>

inline pair<State, State> parseInput(std::istream& input) {
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

int main() {
    auto state = parseInput(std::cin);
    
    std::cout << "Initial State:\n" << state.first << std::endl;
    std::cout << "Goal State:\n" << state.second << std::endl;

    // Create an instance of AStar
    AStar<SlidingPuzzle::State, float> astar;
    
    // Solve using AStar
    auto solution = SlidingPuzzleSolver::solve(state.first, state.second, astar);
    
    if (solution.empty()) {
        std::cout << "No solution found!\n";
    } else {
        std::cout << "Solution found in " << solution.size() - 1 << " moves\n";
    }
    return 0;
} 