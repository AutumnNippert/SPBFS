#pragma once
#include <vector>
#include <functional>
#include <iostream>

template<typename State, typename Cost = float>
class ProblemInstance {
public:
    State initial_state;

    ProblemInstance() : initial_state() {
        std::cout << "Constructing a ProblemInstance" << std::endl;
        std::cout << "&" << this << std::endl;
    }
    ~ProblemInstance() = default;

    virtual std::vector<State> getSuccessors(const State& state) = 0;
    virtual Cost heuristic(const State& state, const State& goal) = 0;
    virtual Cost getCost(const State& state, const State& successor) = 0;
    virtual size_t hash(const State& state) = 0;
}; 