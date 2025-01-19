#pragma once
#include "search.hpp"

#include <boost/unordered/unordered_flat_map.hpp>
using boost::unordered_flat_map;

#include <queue>
#include <boost/heap/d_ary_heap.hpp>

#include <algorithm>
#include <vector>
#include <iostream>

#include <atomic>
#include <mutex>
#include <stop_token>

using std::lock_guard;
using std::mutex;

using namespace std;

template<typename State, typename Cost = float>
class SPAStar : public Search<State, Cost> {
    using GetSuccessors = typename Search<State, Cost>::GetSuccessors;
    using Heuristic = typename Search<State, Cost>::Heuristic;
    using GetCost = typename Search<State, Cost>::GetCost;
    using HashFn = typename Search<State, Cost>::HashFn;

    struct Node;
    struct NodeCompare;
    using MinHeap = boost::heap::d_ary_heap<Node*, boost::heap::arity<2>, boost::heap::mutable_<true>, boost::heap::compare<NodeCompare>>;

public:
    SPAStar(const ProblemInstance<State, Cost>* problemInstance, size_t extra_expansion_time, size_t threadCount) : Search<State, Cost>(problemInstance){
        open = MinHeap();
        closed = unordered_flat_map<State, Node*, HashFn>(0,     
        [this](const State& state) {
            return this->hash(state);
        });
        this->extra_expansion_time = extra_expansion_time;
        this->threadCount = threadCount;
    }
    SPAStar(const ProblemInstance<State, Cost>* problemInstance) : SPAStar(problemInstance, 0, 0) {}

    vector<State> findPath() override {
        this->start();
        nodes.reserve(10'000'000); // reserve 10 million nodes
        // Side note, push back is amortized O(1), so we can compare the speed loss of reserve vs push_back at some point

        nodes.emplace_back(this->problemInstance->initial_state,
            0, this->heuristic(this->problemInstance->initial_state), nullptr);
        Node* startNode = &nodes.back();

        closed.emplace(startNode->state, startNode);
        startNode->handle = open.push(startNode);

        // create a stop source
        stop_source stopSource;

        // start this->threadCount threads doing search()
        vector<jthread> threads;
        vector<Node*> finish_states;
        finish_states.resize(this->threadCount);
        for (size_t i = 0; i < this->threadCount; i++) {
            threads.emplace_back(&SPAStar::search, this, stopSource.get_token(), &finish_states[i]);
        }

        // wait finished threads to be > 0
        while (threadsCompleted.load() < 1) {
            this_thread::yield();
        }

        // now we know that at least one thread has finished, so we can request stop    
        stopSource.request_stop();

        // now we wait for all the threads to finish
        for (size_t i = 0; i < threads.size(); i++) {
            threads[i].join();
        }

        // check all the threads finish states to see which has the smallest f while having h = 0
        Node* finish_state = nullptr;
        for (size_t i = 0; i < finish_states.size(); i++) {
            if (finish_states[i] == nullptr) continue;
            if (finish_states[i]->h == 0) {
                if (finish_state == nullptr || finish_states[i]->f < finish_state->f) {
                    finish_state = finish_states[i];
                }
            }
        }
        return finish(finish_state);
    }

    void search(stop_token st, Node** finish_state) {
        while (!st.stop_requested()) {
            Node* current;
            {
                const lock_guard<mutex> lock(open_mutex); // lock for heap operations
                if (open.empty()) {
                    if(threadsCompleted.load() == this->threadCount-1) {
                        break;
                    }
                    continue;
                }
                current = open.top();
                open.pop();
            }
            if (current->h == 0){
                *finish_state = current; // update the output pointer
                break;
            }
            expand(current);
        }
        threadsCompleted.fetch_add(1);
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
    size_t threadCount = 0;
    mutex open_mutex{};
    mutex closed_mutex{};
    mutex nodes_mutex{};
    atomic<size_t> threadsCompleted{0}; // Track total completed threads

    void expand(Node* n) {
        this->expandedNodes++;
        for (const auto& successorState : this->getSuccessors(n->state)) {
            if (successorState == n->state) continue; // skip the parent state
            this->generatedNodes++;
            this->wasteTime(this->extra_expansion_time);
            // Generate the successor node and calculate its f, g, and h values
            Node* successor;
            {
                const lock_guard<mutex> lock(nodes_mutex);
                nodes.emplace_back(successorState,
                n->g + this->getCost(n->state, successorState),
                this->heuristic(successorState),
                n);
                successor = &nodes.back();
            }
            
            {
                const lock_guard<mutex> lock(closed_mutex);
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
                        {
                            const lock_guard<mutex> lock(open_mutex);
                            open.update(duplicateNode->handle);
                        }
                    }
                    continue; // skip this successor because it's already in closed list and it was already updated
                } else{
                    closed.emplace(successorState, successor);
                    {
                        const lock_guard<mutex> lock(open_mutex);
                        successor->handle = open.push(successor);
                    }
                } 
            }
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