#pragma once
#include <vector>
#include <functional>
#include <iostream>

template<typename State, typename Cost = float>
class Search {
public:
    // Function type definitions
    using GetSuccessors = std::function<std::vector<State>(const State&)>;
    using Heuristic = std::function<Cost(const State&, const State&)>;
    using GetCost = std::function<Cost(const State&, const State&)>;
    using HashFn = std::function<size_t(const State&)>;

    size_t expandedNodes = 0; // Number of nodes expanded during the search
    size_t generatedNodes = 0; // Number of nodes generated during the search
    size_t duplicatedNodes = 0; // Number of nodes duplicated during the search

    State start;
    State goal;
    GetSuccessors getSuccessors;
    Heuristic heuristic;
    GetCost getCost;
    HashFn hash;

    Search(): start(), goal(), getSuccessors(nullptr), heuristic(nullptr), getCost(nullptr), hash(nullptr) {}

    virtual void initialize( // Initialize the search with the given parameters after construction
        const State& start,
        const State& goal,
        GetSuccessors getSuccessors,
        Heuristic heuristic,
        GetCost getCost,
        HashFn hashFunction = nullptr
    ) = 0;


    // Virtual destructor for proper inheritance
    virtual ~Search() = default;

    virtual std::vector<State> findPath() = 0;

    // Function to print search statistics
    void printStats() {
        std::cout << "Expanded nodes: " << expandedNodes << std::endl;
        std::cout << "Generated nodes: " << generatedNodes << std::endl;
        std::cout << "Duplicated nodes: " << duplicatedNodes << std::endl;
        // std::cout << "Max f value: " << maxF << std::endl;
    }
}; 