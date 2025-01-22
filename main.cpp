#include "sliding_puzzle.hpp"
#include "path_finding.hpp"
#include "astar.hpp"
#include "cafe.hpp"
#include "kbfs.hpp"
#include "spastar.hpp"

#include <iostream>

#include <getopt.h>

template <typename State>
void print_path(const std::vector<State>& path) {
    for (const auto& state : path) {
        std::clog << state << std::endl;
    }
}

int main(int argc, char* argv[]) {
    int c;
    std::string algorithmChoice = "astar"; // Default algorithm
    std::string problem = "tiles"; // Default problem
    size_t extraExpansionTime = 0; // Default extra expansion time
    size_t threadCount = 1; // Default thread count

    static struct option long_options[] =
    {
        {"algorithm", required_argument, 0, 'a'},
        {"problem", required_argument, 0, 'p'},
        {"extra-expansion-time", required_argument, 0, 'e'},
        {"threads", required_argument, 0, 't'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    while ((c = getopt_long(argc, argv, "a:p:e:t:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'a':
                algorithmChoice = optarg;
                break;
            case 'p':
                problem = optarg;
                break;
            case 'e':
                extraExpansionTime = std::stoi(optarg); // Convert string to int
                break;
            case 't':
                threadCount = std::stoi(optarg); // Convert string to int
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-a <algorithm>] [-p <problem>] [-e <extra-expansion-time>] [-t <threads>]" << std::endl;
                return 1;
        }
    }
    
    // std::clog << "Algorithm: " << algorithmChoice << std::endl;
    std::clog << "Problem: " << problem << std::endl;
    // std::clog << "Extra expansion time: " << extraExpansionTime << std::endl;
    // std::clog << "Thread count: " << threadCount << std::endl;

    if (algorithmChoice == "astar") {
        if (problem == "tiles") {
            using namespace SlidingPuzzle;
            auto instance = SlidingTileInstance<State>::parseInput(std::cin);
            AStar<State> searcher(&instance, extraExpansionTime);
            auto path = searcher.findPath();
            // print_path(path);
        } else if (problem == "path") {
            using namespace Pathfinding;
            auto instance = PathfindingInstance<State>::parseInput(std::cin);
            AStar<State> searcher(&instance, extraExpansionTime);
            auto path = searcher.findPath();
            // print_path(path);
        }
    } else if (algorithmChoice == "cafe") {
        if (problem == "tiles") {
            using namespace SlidingPuzzle;
            auto instance = SlidingTileInstance<State>::parseInput(std::cin);
            CAFE<State> searcher(&instance, extraExpansionTime, threadCount);
            auto path = searcher.findPath();
            // print_path(path);
        } else if (problem == "path") {
            using namespace Pathfinding;
            auto instance = PathfindingInstance<State>::parseInput(std::cin);
            CAFE<State> searcher(&instance, extraExpansionTime, threadCount);
            auto path = searcher.findPath();
            // print_path(path);
        }
    } else if (algorithmChoice == "kbfs") {
        if (problem == "tiles") {
            using namespace SlidingPuzzle;
            auto instance = SlidingTileInstance<State>::parseInput(std::cin);
            KBFS<State> searcher(&instance, extraExpansionTime, threadCount);
            auto path = searcher.findPath();
            // print_path(path);
        } else if (problem == "path") {
            using namespace Pathfinding;
            auto instance = PathfindingInstance<State>::parseInput(std::cin);
            KBFS<State> searcher(&instance, extraExpansionTime, threadCount);
            auto path = searcher.findPath();
            // print_path(path);
        }
    } else if (algorithmChoice == "spastar") {
        if (problem == "tiles") {
            using namespace SlidingPuzzle;
            auto instance = SlidingTileInstance<State>::parseInput(std::cin);
            SPAStar<State> searcher(&instance, extraExpansionTime, threadCount);
            auto path = searcher.findPath();
            // print_path(path);
        } else if (problem == "path") {
            using namespace Pathfinding;
            auto instance = PathfindingInstance<State>::parseInput(std::cin);
            SPAStar<State> searcher(&instance, extraExpansionTime, threadCount);
            auto path = searcher.findPath();
            // print_path(path);
        }
    }
    return 0;
} 