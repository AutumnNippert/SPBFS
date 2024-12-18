#pragma once
#include "search.hpp"
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

template<typename State, typename Cost = float>
class AStar : public Search<State, Cost> {
private:

    using IsGoalFn = typename Search<State, Cost>::IsGoal;
    using GetSuccessorsFn = typename Search<State, Cost>::GetSuccessors;
    using HeuristicFn = typename Search<State, Cost>::Heuristic;
    using GetCostFn = typename Search<State, Cost>::GetCost;
    using HashFn = typename Search<State, Cost>::HashFn;

    size_t fLayer = 0;
    size_t minH = 0;

    struct Node {
        State state;
        Cost f{}, g{}, h{};
        State parent;

        Node() = default;
        Node(State s) : state(s) {}
        bool operator > (const Node& other) const { return f > other.f; }
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
                    closed.erase(it); // remove it because it's worse than the current successor
                } else {
                    continue;
                }
            }

            closed[successorPos] = successor;


            if (successor.f > this->maxF) 
                this->maxF = successor.f;

            // Add successor to open list
            open.push(successor);
            this->generatedNodes++;
        }
        // this->printStats();
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
        HashFn hash
    ) override {
        priority_queue<Node, vector<Node>, greater<>> open;
        unordered_map<State, Node, decltype(hash)> closed(0, hash);

        Node startNode(start);
        startNode.h = heuristic(start, goal);
        startNode.f = startNode.h;
        open.push(startNode);

        std::cout << "Initial heuristic: " << startNode.h << std::endl;
        this->fLayer = startNode.f;
        this->minH = startNode.h;
        
        this->generatedNodes = 1;

        while (!open.empty()) {
            Node current = open.top();
            open.pop();

            if (current.f > this->fLayer) {
                // std::cout << "New f layer reached: " << current.f << std::endl;
                this->fLayer = current.f;
            }

            if (current.h < this->minH) {
                std::cout << "New minimum heuristic found: " << current.h << std::endl;
                this->minH = current.h;
            }

            if (current.h == 0) {
                std::cout << "Goal found: " << std::endl;
                this->printStats();
                return reconstructPath(current, closed);
            }

            closed[current.state] = current;
            expand(open, closed, current, goal, getSuccessors, heuristic, getCost);
        }


        this->printStats();
        return {};
    }
}; 