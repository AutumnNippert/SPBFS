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
    using IsGoalFn = typename Search<State, Cost>::IsGoal;
    using GetSuccessorsFn = typename Search<State, Cost>::GetSuccessors;
    using HeuristicFn = typename Search<State, Cost>::Heuristic;
    using GetCostFn = typename Search<State, Cost>::GetCost;
    using HashFn = typename Search<State, Cost>::HashFn;

public:
    void initialize(
        const State& i_start,
        const State& i_goal,
        GetSuccessorsFn i_getSuccessors,
        HeuristicFn i_heuristic,
        GetCostFn i_getCost,
        HashFn i_hash = nullptr
    ) override {
        this->goal = i_goal;
        this->start = i_start;
        this->getSuccessors = i_getSuccessors;
        this->heuristic = i_heuristic;
        this->getCost = i_getCost;
        this->hash = i_hash;
        open = priority_queue<Node, vector<Node>, greater<>>();
        closed = unordered_map<State, Node, HashFn>(0, this->hash);
    }

    vector<State> findPath() override {

        Node startNode(this->start);
        startNode.h = this->heuristic(this->start, this->goal);
        startNode.f = startNode.h;
        open.push(startNode);

        std::cout << "Initial heuristic: " << startNode.h << std::endl;
        this->fLayer = startNode.f;
        this->minH = startNode.h;

        while (!open.empty()) {
            Node current = open.top();
            open.pop();

            if (current.f > fLayer) {
                // std::cout << "New f layer reached: " << current.f << std::endl;
                fLayer = current.f;
            }

            if (current.h < minH) {
                std::cout << "New minimum heuristic found: " << current.h << std::endl;
                minH = current.h;
            }

            if (current.h == 0) {
                std::cout << "Goal found: " << std::endl;
                this->printStats();
                return reconstructPath(current, closed);
            }

            closed[current.state] = current;
            expand(current);
        }


        this->printStats();
        return {};
    }

private:

    struct Node {
        State state;
        Cost f{}, g{}, h{};
        State parent;

        Node() = default;
        Node(State s) : state(s) {}

        bool operator > (const Node& other) const { 
            if (f == other.f) {
                return g > other.g;
            }
            return f > other.f;
        }
    };

    size_t fLayer = 0;
    size_t minH = 0;

    priority_queue<Node, vector<Node>, greater<>> open;
    unordered_map<State, Node, HashFn> closed;

    void expand(Node n) {
        this->expandedNodes++;
        for (const auto& successorPos : this->getSuccessors(n.state)) {

            Cost tentativeG = n.g + this->getCost(n.state, successorPos);

            Node successor(successorPos);
            successor.g = tentativeG;
            successor.h = this->heuristic(successorPos, this->goal);
            successor.f = n.g + successor.h;
            successor.parent = n.state;

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
}; 