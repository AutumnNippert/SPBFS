#pragma once

#include "problem_instance.hpp"
#include <vector>
#include <functional>
#include <iostream>

template<typename State, typename Cost = float>
class Search {
public:
    // Function type definitions
    using GetSuccessors = std::function<std::vector<State>(const State&)>;
    using Heuristic = std::function<Cost(const State&)>;
    using GetCost = std::function<Cost(const State&, const State&)>;
    using HashFn = std::function<size_t(const State&)>;

    Search() {
        std::cout << "Constructing an empty Search" << std::endl;
    }

    Search(State initial_state, GetSuccessors getSuccessors, Heuristic heuristic, GetCost getCost, HashFn hash) {
        this->initial_state = initial_state;
        this->getSuccessors = getSuccessors;
        this->heuristic = heuristic;
        this->getCost = getCost;
        this->hash = hash;
    }

    State initial_state;
    GetSuccessors getSuccessors;
    Heuristic heuristic;
    GetCost getCost;
    HashFn hash;

    size_t expandedNodes = 0; // Number of nodes expanded during the search
    size_t generatedNodes = 0; // Number of nodes generated during the search
    size_t duplicatedNodes = 0; // Number of nodes duplicated during the search

    

    virtual std::vector<State> findPath() = 0;

    // Function to print search statistics
    void printStats() {
        std::cout << "Expanded nodes: " << expandedNodes << std::endl;
        std::cout << "Generated nodes: " << generatedNodes << std::endl;
        std::cout << "Duplicated nodes: " << duplicatedNodes << std::endl;
        // std::cout << "Max f value: " << maxF << std::endl;
    }
}; 