#pragma once
#include "search.hpp"

#include <boost/unordered/unordered_flat_map.hpp>
using boost::unordered_flat_map;

#include <queue>
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
    AStar(const ProblemInstance<State, Cost>* problemInstance) : Search<State, Cost>(problemInstance){
        open = priority_queue<Node*, vector<Node*>, greater<>>();
        closed = unordered_flat_map<State, Node*, HashFn>(0,     
        [this](const State& state) {
            return this->hash(state);
        });
    }

    vector<State> findPath() override {
        Node* startNode = new Node(this->problemInstance->initial_state);
        startNode->h = this->heuristic(this->problemInstance->initial_state);
        startNode->f = startNode->h;

        closed.emplace(startNode->state, startNode);
        open.push(startNode);

        while (!open.empty()) {
            Node* current = open.top();
            open.pop();
            if (current->h == 0)
                return finish(current);
            expand(current);
        }
        return finish(nullptr);
    }

private:

    struct Node {
        State state;
        Cost f{}, g{}, h{};
        Node* parent;

        Node() = default;
        Node(State s) : state(s), parent(nullptr) {}

        bool operator > (const Node& other) const { 
            if (f == other.f) 
                return g > other.g;
            return f > other.f;
        }
        bool operator < (const Node& other) const { 
            if (f == other.f) 
                return g < other.g;
            return f < other.f;
        }
    };

    priority_queue<Node*, vector<Node*>, greater<>> open;
    unordered_flat_map<State, Node*, HashFn> closed;

    void expand(Node* n) {
        this->expandedNodes++;
        for (const auto& successorState : this->getSuccessors(n->state)) {
            if (successorState == n->state) continue; // skip the parent state
            this->generatedNodes++;
            // Generate the successor node and calculate its f, g, and h values
            Node* successor = new Node(successorState);
            successor->g = n->g + this->getCost(n->state, successorState);
            successor->h = this->heuristic(successorState);
            successor->f = n->g + successor->h;
            successor->parent = n;

            // Check if successor is already in closed list
            auto duplicate = closed.find(successorState);
            if (duplicate != closed.end()) { 
                this->duplicatedNodes++;
                if (duplicate->second->f >= successor->f) {
                    // delete duplicate->second; // delete the worse duplicate
                    // TODO: Things that point to this node should be updated to point to the new node, but maybe not?
                    duplicate->second = successor; // update duplicate because it's worse than the current successor
                }
                continue; // skip this successor because it's already in closed list and it was already updated
            } else 
                closed.emplace(successorState, successor);
            open.push(successor);
        }
    }

    static vector<State> reconstructPath(
        Node* goal
    ) {
        vector<State> path;
        Node* current = goal;
        while (current != nullptr) {
            path.push_back(current->state);
            current = current->parent;
        }
        reverse(path.begin(), path.end());
        return path;
    }

    vector<State> finish(Node* n) {
        if(n == nullptr) {
            cout << "No path found" << endl;
            this->printStats();
            return {};
        }
        cout << "Goal found: " << endl;
        cout << "Path Length: " << n->g << endl;
        this->printStats();
        return reconstructPath(n);
    }
}; 