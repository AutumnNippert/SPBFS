#pragma once
#include <vector>
#include <functional>
#include <iostream>

template<typename State, typename Cost = float>
class ProblemInstance {
public:
    const State initial_state = State();

    ProblemInstance(const State& initial_state) : initial_state(initial_state) {}

    ~ProblemInstance() = default;

    /**
     * Get the successors of a given state
     * @param state The state to get the successors for
     * @return A vector of successor states
     */
    virtual std::vector<State> getSuccessors(const State& state) const = 0;

    /**
     * Get the heuristic value for a given state
     * @param state The state to calculate the heuristic for
     * @return The heuristic value
     */
    virtual Cost heuristic(const State& state) const = 0;

    /**
     * Get the cost of transitioning from one state to another
     * @param state The current state
     * @param successor The successor state
     * @return The cost of transitioning from the current state to the successor
     */
    virtual Cost getCost(const State& state, const State& successor) const = 0;

    /**
     * Get the hash value of a state
     * @param state The state to hash
     * @return The hash value of the state
     */
    virtual size_t hash(const State& state) const = 0;

    /**
     * Get the maximum number of actions that can be taken from a state
     * Used for allocating nodes in the search algorithms that require it
     * @return The maximum number of actions that can be taken from a state
     */
    virtual size_t maxActionCount() const = 0;
}; 