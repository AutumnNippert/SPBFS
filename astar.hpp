#pragma once
#include "search.hpp"
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <vector>

using namespace std;

template<typename State, typename Cost = float>
class AStar : public Search<State, Cost> {
private:
    struct Node {
        State state;
        Cost f{}, g{}, h{};
        State parent;

        Node() = default;
        Node(State s) : state(s) {}
        bool operator>(const Node& other) const { return f > other.f; }
    };

    void expand(
        priority_queue<Node, vector<Node>, greater<>>& open,
        unordered_map<State, Node, typename Search<State, Cost>::HashFn>& closed,
        const Node& current,
        typename Search<State, Cost>::GetSuccessors getSuccessors,
        typename Search<State, Cost>::Heuristic heuristic,
        typename Search<State, Cost>::GetCost getCost
    ) {
        this->expandedNodes++;
        for (const auto& successorPos : getSuccessors(current.state)) {
            if (closed.find(successorPos) != closed.end())
                continue;

            Cost tentativeG = current.g + getCost(current.state, successorPos);

            Node successor(successorPos);
            successor.g = tentativeG;
            successor.h = heuristic(successorPos, successorPos);
            successor.f = successor.g + successor.h;
            successor.parent = current.state;

            if (successor.f > this->maxF) {
                this->maxF = successor.f;
            }

            
            open.push(successor);
            this->generatedNodes++;

            if (open.size() > this->maxQueueSize) {
                this->maxQueueSize = open.size();
            }
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
        typename Search<State, Cost>::IsGoal isGoal,
        typename Search<State, Cost>::GetSuccessors getSuccessors,
        typename Search<State, Cost>::Heuristic heuristic,
        typename Search<State, Cost>::GetCost getCost,
        typename Search<State, Cost>::HashFn hash = nullptr
    ) override {

        std::cout << "Starting A* search" << std::endl;

        priority_queue<Node, vector<Node>, greater<>> open;
        
        if (!hash) {
            throw runtime_error("Hash function must be provided for custom types");
        }
        unordered_map<State, Node, decltype(hash)> closed(0, hash);

        Node startNode(start);
        startNode.h = heuristic(start, start);
        startNode.f = startNode.h;
        open.push(startNode);
        
        this->generatedNodes = 1;
        this->maxQueueSize = 1;

        while (!open.empty()) {
            Node current = open.top();
            open.pop();

            if (isGoal(current.state)) {
                // std::cout << "Goal found: " << current.state << std::endl;
                return reconstructPath(current, closed);
            }

            closed[current.state] = current;
            expand(open, closed, current, getSuccessors, heuristic, getCost);
        }

        return {};
    }
}; 