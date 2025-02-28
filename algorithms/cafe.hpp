#pragma once
#include "search.hpp"

#include <boost/unordered/unordered_flat_map.hpp>
using boost::unordered_flat_map;

#include "immutable_circular_queue.hpp"

#include "recent_window_heap.hpp"
const size_t PRE_HEAP_SIZE = 8;

#include <algorithm>
#include <vector>
#include <iostream>

#include <atomic>
#include <thread>
#include <mutex>

using std::lock_guard;
using std::mutex;

using namespace std;

template<typename State, typename Cost = float>
class CAFE : public Search<State, Cost> {
    using GetSuccessors = typename Search<State, Cost>::GetSuccessors;
    using Heuristic = typename Search<State, Cost>::Heuristic;
    using GetCost = typename Search<State, Cost>::GetCost;
    using HashFn = typename Search<State, Cost>::HashFn;

    struct Node;
    struct NodeCompare;
    using d_ary_heap = boost::heap::d_ary_heap<Node*, boost::heap::arity<5>, boost::heap::mutable_<true>, boost::heap::compare<NodeCompare>>;
    using handle_type = typename d_ary_heap::handle_type;

    d_ary_heap open{};
    ImmutableCircularQueue<Node*> openQueue;
    unordered_flat_map<State, Node*, HashFn> closed;
    size_t threadCount;
    atomic<size_t> threadsCompleted{0}; // Track total completed threads

public:
    CAFE(const ProblemInstance<State, Cost>* problemInstance, size_t extra_expansion_time, size_t threadCount) : Search<State, Cost>(problemInstance) {
        closed = unordered_flat_map<State, Node*, HashFn>(0,     
            [this](const State& state) {
                return this->hash(state);
            });
        this->extra_expansion_time = extra_expansion_time;
        this->threadCount = threadCount;
        this->openQueue = ImmutableCircularQueue<Node*>(threadCount);

        this->searchStats["Algorithm"] = "CAFE";
        this->searchStats["Extra Expansion Time"] = extra_expansion_time;
        this->searchStats["Threads"] = threadCount;
    }
    CAFE(const ProblemInstance<State, Cost>* problemInstance, size_t threadCount) : CAFE(problemInstance, 0, threadCount) {}

    
    vector<State> findPath() override {
        this->start();
        nodes.reserve(20'000'000);
        nodes.emplace_back(this->problemInstance->initial_state, 0, this->heuristic(this->problemInstance->initial_state), nullptr);
        Node* startNode = &nodes.back();
        startNode->status.store(Status::UNVISITED, std::memory_order_relaxed);

        vector<jthread> threads;
        stop_source stopSource;
        Node* goal = nullptr;

        closed.emplace(startNode->state, startNode);
        startNode->handle = open.push(startNode);
        openQueue = openQueue.push(startNode);

        vector<vector<Node>> threadNodePools;
        threadNodePools.resize(this->threadCount-1);


        for (size_t i = 0; i < this->threadCount-1; i++) {
            // create a new Node Pool for each thread
            threadNodePools[i].reserve(20'000'000);
            threads.emplace_back(&CAFE::thread_speculate, this, i, stopSource.get_token(), &threadNodePools[i]);
        }
        clog << "Threads Initialized" << endl;

        // While should end when Open is empty and none of the threads are working
        while (true) {
            if (open.empty()){
                // clog << "Open is empty" << endl;
                if (threadsCompleted.load(std::memory_order_relaxed) == this->threadCount-1) {
                    clog << "All threads are done" << endl;
                    break;
                }
                continue;
            }
            Node* current = open.top();
            open.pop();

            if (current->h == 0){
                goal = current;
                break;
            }

            vector<Node*>* computedSuccessors;
            Status expected = Status::UNVISITED;
            if (current->status.compare_exchange_strong(expected, Status::WORKING, 
                                                            std::memory_order_acquire, 
                                                            std::memory_order_acq_rel)) {
                expand(current, nodes);
                this->manualExpandedNodes++;
                current->status.store(Status::DONE, std::memory_order_release);
            } else {
                // wait until its Done
                while (current->status.load(std::memory_order_acquire) != Status::DONE) {
                    this_thread::yield();
                }
            }
            computedSuccessors = &current->successors; // memory address of the successors vector
            
            // add successors to open
            this->expandedNodes++;
            for (Node* successor : *computedSuccessors) {
                this->generatedNodes++;

                // duplicate detection
                auto it = closed.find(successor->state);
                if (it != closed.end()) {
                    if (it->second->f > successor->f) {
                        this->duplicatedNodes++;
                        it->second->g = successor->g;
                        it->second->f = successor->f;
                        it->second->parent = successor->parent;
                        it->second->successors = successor->successors;
                        it->second->status.store(Status::UNVISITED, std::memory_order_release);
                        open.update(it->second->handle);
                    }
                    this->generatedNodes--;
                    continue;
                } else 
                    closed.emplace(successor->state, successor);
                successor->handle = open.push(successor);

                {
                    lock_guard<mutex> lock(mtx);
                    openQueue = openQueue.push(successor);
                }
            }
        }

        // request stop
        clog << "Requesting Stop" << endl;
        stopSource.request_stop();

        // join threads
        for(size_t i = 0; i < threads.size(); i++) {
            threads[i].join();
        }

        this->pathLength = goal->g;
        return finish(goal);
    }

private:
    size_t manualExpandedNodes = 0;
    size_t speculatedNodes = 0;
    vector<Node> nodes;
    mutex mtx{};

    enum Status {
        UNVISITED = 0,
        WORKING = 1,
        DONE = 2
    };

    struct Node {
        handle_type handle;
        State state;
        Cost g{}, h{}, f{};
        Node* parent = nullptr;
        std::atomic<Status> status{Status::UNVISITED};
        vector<Node*> successors;

        // Default constructor
        Node() = default;

        // Constructor for initialisation
        explicit Node(const State& s) : state(s) {}

        Node(const State& s, Cost g, Cost h, Node* parent = nullptr)
            : state(s), g(g), h(h), f(g + h), parent(parent) {}

        Node(Node* n)
            : state(n->state), g(n->g), h(n->h), f(n->f), parent(n->parent) {
            status.store(n->status.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }

        Node(const Node& n) : state(n.state), g(n.g), h(n.h), f(n.f), parent(n.parent) {
            status.store(n.status.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        Node(Node&& n) : state(n.state), g(n.g), h(n.h), f(n.f), parent(n.parent) {
            status.store(n.status.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }
        
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

    struct NodeCompare {
        bool operator()(const Node* a, const Node* b) const {
            return *a > *b;
        }
    };

    void thread_speculate(size_t id, stop_token st, vector<Node>* nodePool) {
        // wait until there is something at the ID
        while (openQueue.size() < id) {
            this_thread::yield();
            if(st.stop_requested()) {
                break;
            }
        }

        // While no stop requested
        while (!st.stop_requested()) {

            optional<Node*> option = openQueue.get(id);
            if (!option.has_value()) {
                this_thread::yield();
                continue;
            }

            Node* n = option.value();

            // Atomically check if the node's status is `UNVISITED` and set it to `WORKING`
            Status expected = Status::UNVISITED;
            if (!n->status.compare_exchange_strong(expected, Status::WORKING, 
                                                std::memory_order_acquire, 
                                                std::memory_order_relaxed)) {
                // If the CAS fails, the status was not `UNVISITED`
                continue;
            }
            expand(n, *nodePool);
            n->status.store(Status::DONE, std::memory_order_release);
            {
                lock_guard<mutex> lock(mtx);
                this->speculatedNodes++;
            }
        }
        threadsCompleted.fetch_add(1, std::memory_order_relaxed); // Notifying that this thread is done
    }

    void expand(Node* n, vector<Node>& nodePool) {
        assert(n->status.load(std::memory_order_acquire) != Status::DONE && "BAD!:: NODE ALREADY COMPLETE"); // Node should not be expanded twice
        auto successors = this->getSuccessors(n->state);
        n->successors.reserve(successors.size());
        for (const auto& successorState : successors) {
            if (successorState == n->state) continue; // skip the parent state

            Cost g = n->g + this->getCost(n->state, successorState);
            Cost h = this->heuristic(successorState);
            Node* parent = n;
            nodePool.emplace_back(successorState, g, h, parent);
            Node* successor = &nodePool.back();
            successor->status.store(Status::UNVISITED, std::memory_order_relaxed);

            n->successors.push_back(successor); // Add the successor* to the parent's list of successors
        }
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
        this->searchStats["Manual Expanded Nodes"] = manualExpandedNodes;
        this->searchStats["Speculated Nodes"] = speculatedNodes;
        this->end();
        if(n == nullptr) {
            clog << "No path found" << endl;
            return {};
        }
        return reconstructPath(n);
    }
};