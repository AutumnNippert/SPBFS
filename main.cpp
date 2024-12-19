#include "sliding_puzzle_solver.hpp"
#include "astar.hpp"

#include <iostream>

#include <getopt.h>

int main(int argc, char* argv[]) {
    int c;
    std::string algorithmChoice = "astar"; // Default algorithm
    std::string problem = "tiles"; // Default problem
    int threadCount = 1; // Default thread count

    static struct option long_options[] =
    {
        {"algorithm", required_argument, 0, 'a'},
        {"problem", required_argument, 0, 'p'},
        {"threads", required_argument, 0, 't'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    while ((c = getopt_long(argc, argv, "a:p:t:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'a':
                algorithmChoice = optarg;
                break;
            case 'p':
                problem = optarg;
                break;
            case 't':
                threadCount = std::stoi(optarg); // Convert string to int
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-a <algorithm>] [-p <problem>] [-t <thread count>]" << std::endl;
                return 1;
        }
    }

    Search<SlidingPuzzle::State, float>* searcher;
    if (algorithmChoice == "greedy") {
        // searcher = new Greedy<SlidingPuzzle::State, float>();
    } else if (algorithmChoice == "astar") {
        searcher = new AStar<SlidingPuzzle::State, float>();
    } else {
        std::cerr << "Invalid algorithm choice. Please choose 'greedy' or 'astar'." << std::endl;
        return 1;
    }

    if (searcher == nullptr) {
        std::cerr << "Searcher is uninitialized." << std::endl;
        return 1;
    }

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