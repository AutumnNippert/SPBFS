#pragma once
#include "search.hpp"

#include <boost/unordered/unordered_flat_map.hpp>
using boost::unordered_flat_map;

#include <queue>
#include <boost/heap/d_ary_heap.hpp>

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

    struct Node;
    struct NodeCompare;
    using MinHeap = boost::heap::d_ary_heap<Node*, boost::heap::arity<2>, boost::heap::mutable_<true>, boost::heap::compare<NodeCompare>>;

public:
    AStar(const ProblemInstance<State, Cost>* problemInstance) : Search<State, Cost>(problemInstance){
        open = MinHeap();
        closed = unordered_flat_map<State, Node*, HashFn>(0,     
        [this](const State& state) {
            return this->hash(state);
        });
    }

    vector<State> findPath() override {
        nodes.reserve(10'000'000); // reserve 10 million nodes
        // Side note, push back is amortized O(1), so we can compare the speed loss of reserve vs push_back at some point

        nodes.emplace_back(this->problemInstance->initial_state,
            0, this->heuristic(this->problemInstance->initial_state), nullptr);
        Node* startNode = &nodes.back();

        closed.emplace(startNode->state, startNode);
        startNode->handle = open.push(startNode);

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
        MinHeap::handle_type handle;
        const State state;
        Cost f{}, g{}, h{};
        Node* parent;

        Node() = default;
        Node(State s) : state(s), parent(nullptr) {}
        Node(State s, Cost g, Cost h, Node* parent) : state(s), g(g), h(h), parent(parent) {
            f = g + h;
        }

        bool operator > (const Node& other) const { 
            if (f == other.f) 
                return g > other.g;
            return f > other.f;
        }
    };

    struct NodeCompare {
        bool operator()(const Node* a, const Node* b) const {
            return *a > *b;
        }
    };

    vector<Node> nodes;
    MinHeap open;
    unordered_flat_map<State, Node*, HashFn> closed;

    void expand(Node* n) {
        this->expandedNodes++;
        for (const auto& successorState : this->getSuccessors(n->state)) {
            if (successorState == n->state) continue; // skip the parent state
            this->generatedNodes++;
            // Generate the successor node and calculate its f, g, and h values
            nodes.emplace_back(successorState,
                n->g + this->getCost(n->state, successorState),
                this->heuristic(successorState),
                n);
            Node* successor = &nodes.back();

            // Check if successor is already in closed list
            auto duplicate = closed.find(successorState);
            if (duplicate != closed.end()) { 
                Node* duplicateNode = duplicate->second;
                this->duplicatedNodes++;
                if (duplicateNode->f > successor->f) { // only > because less effort to skip if they have the same f value
                    duplicateNode->g = successor->g;
                    // h should be the same because it's the same state
                    duplicateNode->f = successor->f;
                    duplicateNode->parent = successor->parent;
                    open.update(duplicateNode->handle);
                }
                continue; // skip this successor because it's already in closed list and it was already updated
            } else 
                closed.emplace(successorState, successor);
            successor->handle = open.push(successor);
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