#pragma once
#include "search.hpp"
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

template<typename State, typename Cost = float>
class Greedy : public Search<State, Cost> {
private:

    using IsGoalFn = typename Search<State, Cost>::IsGoal;
    using GetSuccessorsFn = typename Search<State, Cost>::GetSuccessors;
    using HeuristicFn = typename Search<State, Cost>::Heuristic;
    using GetCostFn = typename Search<State, Cost>::GetCost;
    using HashFn = typename Search<State, Cost>::HashFn;

    struct Node {
        State state;
        Cost f{}, g{}, h{};
        State parent;

        Node() = default;
        Node(State s) : state(s) {}
        bool operator>(const Node& other) const { return h > other.h; }
    };

    void expand(
        priority_queue<Node, vector<Node>, greater<>>& open,
        unordered_map<State, Node, HashFn>& closed,
        const Node& current,
        const State& goal,
        GetSuccessorsFn getSuccessors,
        HeuristicFn heuristic,
        GetCostFn getCost
    ) {
        this->expandedNodes++;
        cout << "Expanding node with h value of " << current.h << endl;
        for (const auto& successorPos : getSuccessors(current.state)) {

            Cost tentativeG = current.g + getCost(current.state, successorPos);

            Node successor(successorPos);
            successor.g = tentativeG;
            successor.h = heuristic(successorPos, goal);
            successor.f = successor.g + successor.h;
            successor.parent = current.state;

            // Check if successor is already in closed list
            auto it = closed.find(successorPos);
            if (it != closed.end()) { 
                this->duplicatedNodes++;
                if (it->second.f >= successor.f) {
                    closed.erase(it);
                } else {
                    continue;
                }
            }

            closed[successorPos] = successor;

            // Add successor to open list
            open.push(successor);
            this->generatedNodes++;
        }
        this->printStats();
    }

    static vector<State> reconstructPath(
        const Node& goal,
        const unordered_map<State, Node, typename Search<State, Cost>::HashFn>& closed
    ) {
        vector<State> path;
        State current = goal.state;
        
        while (closed.find(current) != closed.end()) {
            path.push_back(current);
            current = closed.at(current).parent;
        }

        reverse(path.begin(), path.end());
        return path;
    }

public:
    vector<State> findPath(
        const State& start,
        const State& goal,
        GetSuccessorsFn getSuccessors,
        HeuristicFn heuristic,
        GetCostFn getCost,
        HashFn hash = nullptr
    ) override {

        std::cout << "Starting A* search" << std::endl;

        priority_queue<Node, vector<Node>, greater<>> open;
        
        if (!hash) {
            throw runtime_error("Hash function must be provided for custom types");
        }
        unordered_map<State, Node, decltype(hash)> closed(0, hash);

        Node startNode(start);
        startNode.h = heuristic(start, goal);
        startNode.f = startNode.h;
        open.push(startNode);
        
        this->generatedNodes = 1;

        while (!open.empty()) {
            Node current = open.top();
            open.pop();

            if (current.h == 0) {
                // std::cout << "Goal found: " << current.state << std::endl;
                return reconstructPath(current, closed);
            }

            closed[current.state] = current;
            expand(open, closed, current, goal, getSuccessors, heuristic, getCost);
        }

        return {};
    }
}; 