#pragma once
#include "search.hpp"

#include <boost/unordered/unordered_flat_map.hpp>
using boost::unordered_flat_map;

#include "array_heap.hpp"
const size_t PRE_HEAP_SIZE = 32;

#include <algorithm>
#include <vector>
#include <iostream>
#include <optional>

#include <atomic>
#include <thread>

using namespace std;

template<typename State, typename Cost = float>
class CAFE : public Search<State, Cost> {
    using GetSuccessors = typename Search<State, Cost>::GetSuccessors;
    using Heuristic = typename Search<State, Cost>::Heuristic;
    using GetCost = typename Search<State, Cost>::GetCost;
    using HashFn = typename Search<State, Cost>::HashFn;

    struct Node;
    ArrayHeap<Node*> open;
    unordered_flat_map<State, Node*, HashFn> closed;
    size_t threadCount;
    atomic<size_t> threadsCompleted{0}; // Track total completed threads

public:
    CAFE(const ProblemInstance<State, Cost>* problemInstance, size_t threadCount) : Search<State, Cost>(problemInstance) {
        open = ArrayHeap<Node*>(PRE_HEAP_SIZE);
        closed = unordered_flat_map<State, Node*, HashFn>(0,     
            [this](const State& state) {
                return this->hash(state);
            });
        this->threadCount = threadCount;
    }

    
    vector<State> findPath() override {
        this->start();

        Node* startNode = new Node(this->problemInstance->initial_state);
        startNode->h = this->heuristic(this->problemInstance->initial_state);
        startNode->f = startNode->h;

        vector<jthread> threads;
        stop_source stopSource;
        Node* goal = nullptr;

        closed.emplace(startNode->state, startNode);
        open.push(startNode);

        for (size_t i = 0; i < this->threadCount; i++) {
            threads.emplace_back(&CAFE::thread_speculate, this, i, stopSource.get_token());
        }
        cout << "Threads Initialized" << endl;

        // While should end when Open is empty and none of the threads are working,
        while (!open.empty() || threadsCompleted.load() < this->threadCount) {
            auto option = open.pop();
            Node* current = option.value(); // should always have a value if open is not empty

            if (current->h == 0){
                stopSource.request_stop();
                goal = current;
                break;
            }

            vector<Node*>* computedSuccessors;

            if (current->status != Status::DONE) { // if not done, do it manually
                expand(current);
                this->manualExpandedNodes++;
            }
            computedSuccessors = &current->successors;
            
            // add successors to open
            for (Node* successor : *computedSuccessors) {
                this->generatedNodes++;

                // duplicate detection
                auto it = closed.find(successor->state);
                if (it != closed.end()) {
                    this->duplicatedNodes++;
                    if (it->second->f > successor->f) {
                        it->second = successor;
                    }
                    continue;
                }
                
                closed.emplace(successor->state, successor);
                open.push(successor);
            }
        }

        // join threads
        for (auto& thread : threads) {
            thread.join();
        }
        return finish(goal);
    }

private:
    size_t manualExpandedNodes = 0;
    size_t speculatedNodes = 0;

    enum Status {
        UNVISITED = 0,
        WORKING = 1,
        DONE = 2
    };

    struct Node {
        State state;
        Cost f{}, g{}, h{};
        Node* parent;
        std::atomic<uint8_t> status; // 0 = unvisited, 1 = working, 2 = done
        vector<Node*> successors;

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

    void thread_speculate(size_t id, stop_token st) {
        // wait until there is something at the ID
        while (id < open.size()) {
            this_thread::yield();
        }

        // While the stop token is not requested or there are still nodes to expand
        while (!st.stop_requested() || !open.empty() || threadsCompleted.load() < this->threadCount) {
            optional<Node*> option = open.get(id);
            if (!option.has_value()) {
                this_thread::yield();
                continue;
            }
            Node* n = option.value();
            expand(n);
            speculatedNodes++;
        }
        threadsCompleted.fetch_add(1, std::memory_order_relaxed); // Notifying that this thread is done
    }

    void expand(Node* n) {
        if (n->status != Status::UNVISITED) return;
        n->status = Status::WORKING;
        this->expandedNodes++;
        auto successors = this->getSuccessors(n->state);
        n->successors.reserve(successors.size());
        for (const auto& successorState : successors) {
            if (successorState == n->state) continue; // skip the parent state
            this->generatedNodes++;

            Node* successor = new Node(successorState);
            successor->g = n->g + this->getCost(n->state, successorState);
            successor->h = this->heuristic(successorState);
            successor->f = n->g + successor->h;
            successor->parent = n;

            n->successors.push_back(successor); // Add the successor to the parent's list of successors
        }
        n->status = Status::DONE;
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
        cout << "Manual Expanded Nodes: " << manualExpandedNodes << endl;
        cout << "Speculated Nodes: " << speculatedNodes << endl;
        if(n == nullptr) {
            cout << "No path found" << endl;
            return {};
        }
        cout << "Goal found: " << endl;
        cout << "Path Length: " << n->g << endl;
        return reconstructPath(n);
    }
}; 