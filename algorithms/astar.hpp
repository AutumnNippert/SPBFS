#pragma once
#include "search.hpp"
#include <queue>
// #include <unordered_map>
#include <boost/unordered/unordered_flat_map.hpp>
#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

template<typename State, typename Cost = float>
class AStar : public Search<State, Cost> {
    using GetSuccessors = typename Search<State, Cost>::GetSuccessors;
    using Heuristic = typename Search<State, Cost>::Heuristic;
    using GetCost = typename Search<State, Cost>::GetCost;
    using HashFn = typename Search<State, Cost>::HashFn;

public:
    void initialize(
        const State& i_start,
        const State& i_goal,
        GetSuccessors i_getSuccessors,
        Heuristic i_heuristic,
        GetCost i_getCost,
        HashFn i_hash = nullptr
    ) override {
        this->goal = i_goal;
        this->start = i_start;
        this->getSuccessors = i_getSuccessors;
        this->heuristic = i_heuristic;
        this->getCost = i_getCost;
        this->hash = i_hash;
        open = priority_queue<Node, vector<Node>, greater<>>();
        closed = boost::unordered_flat_map<State, Node, HashFn>(0, this->hash);
    }

    vector<State> findPath() override {

        Node startNode(this->start);
        startNode.h = this->heuristic(this->start, this->goal);
        startNode.f = startNode.h;
        open.push(startNode);

        cout << "Initial heuristic: " << endl << startNode.h << endl;
        this->fLayer = startNode.f;
        this->minH = startNode.h;

        cout << "Initial state: " << this->start.toString() << endl;

        while (!open.empty()) {
            Node current = open.top();
            open.pop();

            if (current.f > fLayer) {
                // std::cout << "New f layer reached: " << current.f << std::endl;
                fLayer = current.f;
            }

            if (current.h < minH) {
                // cout << "New minimum heuristic found: " << current.h << endl;
                minH = current.h;
            }

            if (current.h == 0) {
                cout << "Goal found: " << endl;
                cout << "Path Length: " << current.g << endl;
                this->printStats();
                return reconstructPath(current, closed);
            }

            closed.emplace(current.state, current);
            expand(current);
        }

        cout << "No path found" << endl;
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
    boost::unordered_flat_map<State, Node, HashFn> closed;

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
                if (it->second.f >= successor.f) it->second = successor; // update it because it's worse than the current successor
                else continue;
            } else closed.emplace(successorPos, successor);

            // Add successor to open list
            open.push(successor);
            this->generatedNodes++;
        }
        // this->printStats();
    }

    static vector<State> reconstructPath(
        const Node& goal,
        const boost::unordered_flat_map<State, Node, HashFn>& closed
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