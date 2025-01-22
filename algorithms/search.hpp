#pragma once

#include "problem_instance.hpp"
#include <vector>
#include <functional>
#include <iostream>
#include <chrono>

// Stat Recording
#include <string>
#include <map>
#include <variant>
#include <type_traits>

using Value = std::variant<int, long int, size_t, double, bool, std::string>;

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

    // clock
    std::chrono::high_resolution_clock::time_point clockStart;

    size_t extra_expansion_time = 0;

    size_t expandedNodes = 0; // Number of nodes expanded during the search
    size_t generatedNodes = 0; // Number of nodes generated during the search
    size_t duplicatedNodes = 0; // Number of nodes duplicated during the search
    
    long pathLength = -1; // Length of the path found
    
    // create a map of string to string to store the search statistics
    std::map<std::string, Value> searchStats;

    const ProblemInstance<State, Cost> *problemInstance;
    
    inline std::vector<State> getSuccessors(const State& state) const { return problemInstance->getSuccessors(state); }
    inline Cost heuristic(const State& state) const { return problemInstance->heuristic(state); }
    inline Cost getCost(const State& state, const State& successor) const { return problemInstance->getCost(state, successor); }
    inline size_t hash(const State& state) const { return problemInstance->hash(state); }

    virtual std::vector<State> findPath() = 0;

    inline void wasteTime(std::size_t n){
        std::size_t s_i = 0;
        volatile std::size_t * sum_i = &s_i;
        for(std::size_t j = 0; j < n; j++){
            *sum_i += j;
        }
    }

    inline std::string toJsonValue(const Value& v) {
        return std::visit([](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                // Strings should be in quotes
                return "\"" + arg + "\"";
            } else if constexpr (std::is_same_v<T, bool>) {
                // Booleans -> true/false
                return arg ? "true" : "false";
            } else {
                // Numeric types -> as is
                return std::to_string(arg);
            }
        }, v);
    }

    // Function to print search statistics in a json format
    void printStats() {
        // Print them as JSON
        std::cout << "{\n";
        for (auto it = searchStats.begin(); it != searchStats.end(); ++it) {
            std::cout << "  \"" << it->first << "\": " << toJsonValue(it->second);
            if (std::next(it) != searchStats.end()) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "}\n";
    }

    void start() {
        this->clockStart = std::chrono::high_resolution_clock::now();
    }

    void end() {
        auto clockEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = clockEnd - this->clockStart;

        // Add the search stats to the map
        searchStats["Expanded Nodes"] = expandedNodes;
        searchStats["Generated Nodes"] = generatedNodes;
        searchStats["Duplicated Nodes"] = duplicatedNodes;
        searchStats["Extra Expansion Time"] = extra_expansion_time;
        searchStats["Elapsed Time"] = elapsed.count();
        searchStats["Path Length"] = pathLength;

        this->printStats();
    }
}; 