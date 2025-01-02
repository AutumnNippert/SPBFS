// #include "sliding_puzzle.hpp"
#include "path_finding.hpp"
#include "astar.hpp"

#include <iostream>

#include <getopt.h>

template <typename State>
void print_path(const std::vector<State>& path) {
    for (const auto& state : path) {
        std::cout << state << std::endl;
    }
}

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
    
    std::cout << "Algorithm: " << algorithmChoice << std::endl;
    std::cout << "Problem: " << problem << std::endl;
    std::cout << "Thread count: " << threadCount << std::endl;

    if (problem == "tiles") {
        // using namespace SlidingPuzzle;
        // auto [initial, goal] = SlidingPuzzleSolver::parseInput(std::cin);
        // AStar<State> searcher;
        // searcher.initialize(initial, goal, SlidingPuzzleSolver::getSuccessors, SlidingPuzzleSolver::manhattanDistance, SlidingPuzzleSolver::getCost, SlidingPuzzleSolver::hash);
        // auto path = searcher.findPath();
        // print_path(path);
    } else if (problem == "path") {
        using namespace Pathfinding;
        auto instance = PathfindingInstance<State>::parseInput(std::cin);
        AStar<State> searcher(instance);
        auto path = searcher.findPath();
        // print_path(path);
    }
    return 0;
} 