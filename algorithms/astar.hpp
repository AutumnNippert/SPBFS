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
    AStar(State initial_state, GetSuccessors getSuccessors, Heuristic heuristic, GetCost getCost, HashFn hash) : Search<State, Cost>(initial_state, getSuccessors, heuristic, getCost, hash) {
        open = priority_queue<Node, vector<Node>, greater<>>();
        closed = boost::unordered_flat_map<State, Node, HashFn>(0, hash);
    }

    vector<State> findPath() override {

        Node startNode(this->initial_state);
        startNode.h = this->heuristic(this->initial_state);
        startNode.f = startNode.h;
        open.push(startNode);

        cout << "Initial heuristic: " << endl << startNode.h << endl;
        this->fLayer = startNode.f;
        this->minH = startNode.h;

        cout << "Initial state: " << this->initial_state.toString() << endl;

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
                return reconstructPath(startNode, current);
            }

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
        Node *parent;

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
            successor.h = this->heuristic(successorPos);
            successor.f = n.g + successor.h;
            successor.parent = const_cast<Node*>(&n); // const_cast is used to remove the const qualifier so we can modify the parent pointer

            // Check if successor is already in closed list
            auto duplicate = closed.find(successorPos);
            if (duplicate != closed.end()) { 
                this->duplicatedNodes++;
                if (duplicate->second.f >= successor.f){
                    duplicate->second = successor; // update it because it's worse than the current successor
                }
                else continue;
            } else closed.emplace(successorPos, successor);

            // Add successor to open list
            open.push(successor);
            this->generatedNodes++;
        }
        // this->printStats();
    }

    static vector<State> reconstructPath(
        const Node& start_node,
        const Node& goal_node
    ) {
        vector<State> path;
        const Node* current = &goal_node;
        while (current != nullptr) {
            path.push_back(current->state);
            current = current->parent;
        }
        reverse(path.begin(), path.end());
        return path;
    }
}; 