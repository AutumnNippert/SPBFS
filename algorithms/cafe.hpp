#pragma once
#include "search.hpp"

#include <boost/unordered/unordered_flat_map.hpp>
using boost::unordered_flat_map;

#include "mutable_circular_queue.hpp"
#include "immutable_circular_queue.hpp"

#include "recent_window_heap.hpp"
const size_t PRE_HEAP_SIZE = 4;

#include <algorithm>
#include <vector>
#include <iostream>
#include <optional>

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
    using d_ary_heap = boost::heap::d_ary_heap<Node*, boost::heap::arity<2>, boost::heap::mutable_<true>, boost::heap::compare<NodeCompare>>;
    using handle_type = typename d_ary_heap::handle_type;

    d_ary_heap open{};
    ImmutableCircularQueue<Node*, PRE_HEAP_SIZE> openQueue;
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
    }
    CAFE(const ProblemInstance<State, Cost>* problemInstance, size_t threadCount) : CAFE(problemInstance, 0, threadCount) {}

    
    vector<State> findPath() override {
        this->start();
        nodes.reserve(10'000'000);
        nodes.emplace_back(this->problemInstance->initial_state, 0, this->heuristic(this->problemInstance->initial_state), nullptr);
        Node* startNode = &nodes.back();
        startNode->status.store(Status::UNVISITED, std::memory_order_relaxed);

        vector<jthread> threads;
        stop_source stopSource;
        Node* goal = nullptr;

        closed.emplace(startNode->state, startNode);
        startNode->handle = open.push(startNode);
        openQueue = openQueue.push(startNode);

        for (size_t i = 0; i < this->threadCount; i++) {
            threads.emplace_back(&CAFE::thread_speculate, this, i, stopSource.get_token());
        }
        cout << "Threads Initialized" << endl;

        // While should end when Open is empty and none of the threads are working,
        // or when open is empty and the thread count is 0
        while (true) {
            if (open.empty()){
                // cout << "Open is empty" << endl;
                if (threadsCompleted.load(std::memory_order_relaxed) == this->threadCount) {
                    cout << "All threads are done" << endl;
                    break;
                }
                continue;
            }
            Node* current = open.top();
            open.pop();

            if (current->h == 0){
                cout << "Requesting Stop" << endl;
                stopSource.request_stop();
                goal = current;
                break;
            }

            vector<Node*>* computedSuccessors;
            bool speculated = false;
            Status expected = Status::UNVISITED;
            if (current->status.compare_exchange_strong(expected, Status::WORKING, 
                                                            std::memory_order_acquire, 
                                                            std::memory_order_relaxed)) {       
                // cout << "Manual Expansion" << endl;
                expand(current, nodes);
                this->manualExpandedNodes++;
                current->status.store(Status::DONE, std::memory_order_release);
            } else {
                // cout << "Speculated Expansion" << endl;
                speculated = true;
                // wait until its Done
                while (current->status.load(std::memory_order_acquire) != Status::DONE) {
                    // cout << "Waiting for Done" << endl;
                    this_thread::yield();
                }
            }
            computedSuccessors = &current->successors; // memory address of the successors vector
            
            // add successors to open
            // if(speculated) cout << "From Speculated Nodes ";
            // cout << "Adding " << computedSuccessors->size() << " Successors to Open" << endl;
            for (Node* successor : *computedSuccessors) {
                // if(speculated) cout << "\t\t" << successor->state << endl;
                this->generatedNodes++;

                // duplicate detection
                auto it = closed.find(successor->state);
                if (it != closed.end()) {
                    this->duplicatedNodes++;
                    if (it->second->f > successor->f) {
                        it->second->g = successor->g;
                        it->second->f = successor->f;
                        it->second->parent = successor->parent;
                        open.update(it->second->handle);
                    }
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
        cout << "Requesting Stop, exiting while loop" << endl;
        stopSource.request_stop();

        // join threads
        for(size_t i = 0; i < threads.size(); i++) {
            threads[i].join();
            cout << "Thread " << i << " joined" << endl;
        }
        cout << "All threads joined" << endl;
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
        std::atomic<Status> status; // 0 = unvisited, 1 = working, 2 = done
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

    // ISSUE: Once other threads are done, the nodes they have posession of may be being expanded by this thread, causing invalid read of memory
    void thread_speculate(size_t id, stop_token st) {
        cout << "Thread " << id << " is starting" << endl;
        vector<Node> nodePool;
        nodePool.reserve(10'000'000);

        // wait until there is something at the ID
        while (openQueue.size() < id) {
            this_thread::yield();
            if(st.stop_requested()) {
                cout << "Thread " << id << " is stopping" << endl;
                break;
            }
        }

        cout << "Thread " << id << " released" << endl;

        // While there are still nodes to expand
        while (!openQueue.empty()) {
            if(st.stop_requested()) {
                cout << "Thread " << id << " is stopping" << endl;
                break;
            }

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
            // cout << "Thread " << id << " is expanding" << endl;
            expand(n, nodePool);
            n->status.store(Status::DONE, std::memory_order_release);
            {
                lock_guard<mutex> lock(mtx); // for adding to speculatedNodes
                this->speculatedNodes++;
            }
        }
        cout << "Thread " << id << " is done" << endl;
        threadsCompleted.fetch_add(1, std::memory_order_relaxed); // Notifying that this thread is done
    }

    void expand(Node* n, vector<Node>& nodePool) {
        this->expandedNodes++;
        if(n->status.load(std::memory_order_acquire) == Status::DONE) {
            cout << "BAD!:: NODE ALREADY COMPLETE" << endl;
            return;
        }
        auto successors = this->getSuccessors(n->state);
        n->successors.reserve(successors.size());
        for (const auto& successorState : successors) {
            // cout << "\tExpanding Successor" << endl;
            if (successorState == n->state) continue; // skip the parent state

            Cost g = n->g + this->getCost(n->state, successorState);
            Cost h = this->heuristic(successorState);
            Node* parent = n;
            nodePool.emplace_back(successorState, g, h, parent);
            Node* successor = &nodePool.back();
            successor->status.store(Status::UNVISITED, std::memory_order_relaxed);

            n->successors.push_back(successor); // Add the successor to the parent's list of successors
        }
        this->wasteTime(this->extra_expansion_time);
        // cout << "\tNode Expanded" << endl;
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

    void lock(mutex& mtx, string name) {
        cout << name << "::lock" << endl;
        mtx.lock();
        cout << name << "::successfully locked" << endl;

    }

    void unlock(mutex& mtx, string name) {
        cout << name << "::unlock" << endl;
        mtx.unlock();
        cout << name << "::successfully unlocked" << endl;
    }
};