// main thread gives out k threads to work on the problem

#pragma once
#include "search.hpp"

#include <boost/unordered/unordered_flat_map.hpp>
using boost::unordered_flat_map;

#include <queue>
#include <boost/heap/d_ary_heap.hpp>

#include <algorithm>
#include <vector>
#include <iostream>

#include <thread>
#include <barrier>

using namespace std;

template<typename State, typename Cost = float>
class KBFS : public Search<State, Cost> {
    using GetSuccessors = typename Search<State, Cost>::GetSuccessors;
    using Heuristic = typename Search<State, Cost>::Heuristic;
    using GetCost = typename Search<State, Cost>::GetCost;
    using HashFn = typename Search<State, Cost>::HashFn;

    size_t threadCount;
    struct Node;
    struct NodeCompare;
    using d_ary_heap = boost::heap::d_ary_heap<Node*, boost::heap::arity<2>, boost::heap::mutable_<true>, boost::heap::compare<NodeCompare>>;


public:
    KBFS(const ProblemInstance<State, Cost>* problemInstance, size_t extra_expansion_time, size_t threadCount) : Search<State, Cost>(problemInstance){
        open = d_ary_heap();
        closed = unordered_flat_map<State, Node*, HashFn>(0,     
        [this](const State& state) {
            return this->hash(state);
        });
        this->extra_expansion_time = extra_expansion_time;
        this->threadCount = threadCount;
    }
    KBFS(const ProblemInstance<State, Cost>* problemInstance, size_t threadCount) : KBFS(problemInstance, 0, threadCount) {}

    vector<State> findPath() override {
        this->start();
        nodes.reserve(10'000'000);
        nodes.emplace_back(this->problemInstance->initial_state,
            0, this->heuristic(this->problemInstance->initial_state), nullptr);
        Node* startNode = &nodes.back();

        closed.emplace(startNode->state, startNode);
        startNode->handle = open.push(startNode);

        while (!open.empty()) {
            vector<Node*> threadNodes;
            vector<jthread> threads;
            
            // Get nodes for the amount of nodes in the open list up to the thread count
            for (size_t i = 0; i < this->threadCount; i++) {
                if (open.empty()) break; // Stops creating threads if the open list is empty
                threadNodes.push_back(open.top());
                Node* popped = threadNodes.back();
                open.pop();
                if (popped->h == 0) {
                    return finish(popped); // if node is goal, return the path
                    // the first node found in the open list is the shortest path unless multiple nodes were expanded, but if h is 0, the node with the smallest g value is the shortest path, which will also have a smaller f value
                }
            }

            vector<vector<Node*>> allSuccessors(threadNodes.size()); // Persistent storage for successors

            for (size_t i = 0; i < threadNodes.size(); i++) {
                if (i >= threadNodes.size()) break;

                // Allocate memory for the successors
                allSuccessors[i].resize(this->problemInstance->maxActionCount()); // second dimension is for each action
                for (size_t j = 0; j < this->problemInstance->maxActionCount(); j++) {
                    nodes.push_back(Node());
                    allSuccessors[i][j] = &nodes.back();
                }

                threads.emplace_back(&KBFS::expand, this, threadNodes[i], ref(allSuccessors[i]));
            }
                        
            // Wait for all threads to finish
            for (size_t i = 0; i < threads.size(); i++) {
                threads[i].join();
            }

            // add the successors to the open list
            // cout << "Processing batch of: " << threadNodes.size() << endl;
            for (size_t i = 0; i < threadNodes.size(); i++) {
                this->expandedNodes++;

                // cout << "Size of successors: " << threadNodes[i]->successors.size() << endl;
                for (size_t j = 0; j < threadNodes[i]->successors.size(); j++) {
                    Node* successor = threadNodes[i]->successors[j];
                    this->generatedNodes++;
                    updateDuplicateIfNeeded(successor);
                }
                threadNodes[i]->successors.clear(); // clear the successors vector for the next iteration
            }
        }
        return finish(nullptr);
    }

private:

    struct Node {
        d_ary_heap::handle_type handle;
        State state;
        Cost f{}, g{}, h{};
        Node* parent;
        vector<Node*> successors;
        size_t num_successors = 0;

        Node() : state(State()), parent(nullptr) {}
        Node(State s) : state(s), parent(nullptr) {}
        Node(State s, Cost g, Cost h, Node* parent) : state(s), g(g), h(h), parent(parent) {
            f = g + h;
        }
        Node(Node* n) : state(n->state), f(n->f), g(n->g), h(n->h), parent(n->parent) {}

        bool operator > (const Node& other) const { 
            if (f == other.f) 
                return g > other.g;
            return f > other.f;
        }

        //override << 
        friend ostream& operator<<(ostream& os, const Node& n) {
            // print like [f, g, h]
            os << "[" << n.f << ", " << n.g << ", " << n.h << "]";
            return os;
        }
    };
    
    struct NodeCompare {
        bool operator()(const Node* a, const Node* b) const {
            return *a > *b;
        }
    };

    vector<Node> nodes;
    d_ary_heap open;
    unordered_flat_map<State, Node*, HashFn> closed;

    void updateDuplicateIfNeeded(Node* n){
        // check and updates the duplicate node
        auto duplicate = closed.find(n->state);
        if (duplicate != closed.end()) { 
            Node* duplicateNode = duplicate->second;
            this->duplicatedNodes++;
            if (duplicateNode->f >= n->f) {
                duplicateNode->g = n->g;
                // h should be the same because it's the same state
                duplicateNode->f = n->f;
                duplicateNode->parent = n->parent;
                // cout << "Handle location: " << &(duplicateNode->handle) << endl;
                // open.update(duplicateNode->handle);
            }
            return; // skip this successor because it's already in closed list and it was already updated
        } else 
            closed.emplace(n->state, n);
        // cout << *n << endl;
        n->handle = open.push(n);

    }

    // Node** successors is a pointer to an array of pointers to nodes up to the max action count that are allocated prior to calling this function
    void expand(Node* n, vector<Node*>& successors) {
        vector<State> successorStates = this->getSuccessors(n->state);
        for (size_t i = 0; i < successorStates.size(); i++) {
            State successorState = successorStates[i];
            if (successorState == n->state) continue;
            successors[i]->state = successorState;
            successors[i]->g = n->g + this->getCost(n->state, successorState);
            successors[i]->h = this->heuristic(successorState);
            successors[i]->f = successors[i]->g + successors[i]->h;
            successors[i]->parent = n;
            n->num_successors++;
        }
        n->successors = successors;
        n->successors.resize(n->num_successors);
        this->wasteTime(this->extra_expansion_time);
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
        this->end();
        if(n == nullptr) {
            cout << "No path found" << endl;
            return {};
        }
        cout << "Goal found: " << endl;
        cout << "Path Length: " << n->g << endl;
        return reconstructPath(n);
    }
}; 