#pragma once

#include "problem_instance.hpp"
#include <vector>
#include <functional>
#include <iostream>

template<typename State, typename Cost = float>
class Search {
public:

    Search() {
        std::cout << "Constructing an empty Search" << std::endl;
    }

    Search(const ProblemInstance<State, Cost>* problemInstance) : problemInstance(problemInstance) {}

    // Function type definitions
    using GetSuccessors = std::function<std::vector<State>(const State&)>;
    using Heuristic = std::function<Cost(const State&, const State&)>;
    using GetCost = std::function<Cost(const State&, const State&)>;
    using HashFn = std::function<size_t(const State&)>;

    size_t expandedNodes = 0; // Number of nodes expanded during the search
    size_t generatedNodes = 0; // Number of nodes generated during the search
    size_t duplicatedNodes = 0; // Number of nodes duplicated during the search

    const ProblemInstance<State, Cost> *problemInstance;
    
    inline std::vector<State> getSuccessors(const State& state) const { return problemInstance->getSuccessors(state); }
    inline Cost heuristic(const State& state) const { return problemInstance->heuristic(state); }
    inline Cost getCost(const State& state, const State& successor) const { return problemInstance->getCost(state, successor); }
    inline size_t hash(const State& state) const { return problemInstance->hash(state); }

    virtual std::vector<State> findPath() = 0;

    // Function to print search statistics
    void printStats() {
        std::cout << "Expanded nodes: " << expandedNodes << std::endl;
        std::cout << "Generated nodes: " << generatedNodes << std::endl;
        std::cout << "Duplicated nodes: " << duplicatedNodes << std::endl;
        // std::cout << "Max f value: " << maxF << std::endl;
    }
}; 