#pragma once

#include "problem.hpp"
#include <vector>
#include <functional>
#include <iostream>

template<typename State, typename Cost = float>
class Search {
public:

    Search(ProblemInstance<State, Cost>& problemInstance){
        std::cout << "Constructing a Search" << std::endl;
        std::cout << "&" << &problemInstance << std::endl;
        std::cout << problemInstance.initial_state << std::endl;
        this->start = problemInstance.initial_state;
        this->goal = problemInstance.initial_state;
        this->getSuccessors = std::bind(&ProblemInstance<State, Cost>::getSuccessors, &problemInstance, std::placeholders::_1);
        this->heuristic = std::bind(&ProblemInstance<State, Cost>::heuristic, &problemInstance, std::placeholders::_1, std::placeholders::_2);
        this->getCost = std::bind(&ProblemInstance<State, Cost>::getCost, &problemInstance, std::placeholders::_1, std::placeholders::_2);
        this->hash = std::bind(&ProblemInstance<State, Cost>::hash, &problemInstance, std::placeholders::_1);
    }

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

    virtual std::vector<State> findPath() = 0;

    // Function to print search statistics
    void printStats() {
        std::cout << "Expanded nodes: " << expandedNodes << std::endl;
        std::cout << "Generated nodes: " << generatedNodes << std::endl;
        std::cout << "Duplicated nodes: " << duplicatedNodes << std::endl;
        // std::cout << "Max f value: " << maxF << std::endl;
    }
}; 