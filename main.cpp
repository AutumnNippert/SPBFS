#include "sliding_puzzle_solver.hpp"
#include "astar.hpp"
#include "greedy.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <algorithm> (greedy or astar) <problem> (tiles)" << std::endl;
        return 1;
    }
    std::string algorithmChoice = argv[1];

    Search<SlidingPuzzle::State, float>* searcher;
    if (algorithmChoice == "greedy") {
        searcher = new Greedy<SlidingPuzzle::State, float>();
    } else if (algorithmChoice == "astar") {
        searcher = new AStar<SlidingPuzzle::State, float>();
    } else {
        std::cerr << "Invalid algorithm choice. Please choose 'greedy' or 'astar'." << std::endl;
        return 1;
    }

    std::string problem = argv[2];

    if (problem == "tiles") {
        auto instance = SlidingPuzzleSolver::parseInput(std::cin);
        std::cout << "Beginning " << algorithmChoice << " search for sliding puzzle problem\n"; 
        auto solution = SlidingPuzzleSolver::solve(instance.first, instance.second, *searcher);
        if (solution.empty()) {
            std::cout << "No solution found!\n";
        } else {
            std::cout << "Solution found in " << solution.size() - 1 << " moves\n";
        }
    } else {
        std::cerr << "Invalid problem choice. Please choose 'tiles'." << std::endl;
        return 1;
    }


    delete searcher; // Cleanup dynamically allocated memory
    return 0;
} 