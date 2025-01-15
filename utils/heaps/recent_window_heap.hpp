#include "windowed_heap.hpp"

#include <array>
#include <boost/heap/d_ary_heap.hpp>

#include <mutex>

using std::lock_guard;
using std::mutex;

/**
 * RecentWindowHeap is a heap paired with an array that stores copies of the most recent elements in a round robin fashion.
 */
template <typename T, typename Comparator, typename heap_type, typename handle_type, size_t MaxSize>
class RecentWindowHeap {

public:
    RecentWindowHeap(){
        static_assert(MaxSize > 0, "MaxSize must be greater than 0");
    }
    // Disable copy semantics
    RecentWindowHeap(const RecentWindowHeap&) = delete;
    RecentWindowHeap& operator=(const RecentWindowHeap&) = delete;

    // Enable move semantics
    RecentWindowHeap(RecentWindowHeap&& other) noexcept = default;
    RecentWindowHeap& operator=(RecentWindowHeap&& other) noexcept = default;
    
    ~RecentWindowHeap() = default;

    handle_type push(T item) {
        lock_guard<mutex> lock(mtx);
        auto handle = min_heap.push(item);
        items[round_robin_index++] = &item; // store pointer to the item

        // increment array_size if it is less than MaxSize
        if (array_size < MaxSize) {
            array_size++;
        }

        // reset round_robin_index
        if (round_robin_index >= MaxSize) {
            round_robin_index = 0;
        }
        return handle;
    }

    optional<T> pop() {\
        lock_guard<mutex> lock(mtx);
        if(min_heap.empty()) {
            return nullopt;
        }
        auto t = min_heap.top();
        min_heap.pop();
        return t;
    }

    optional<T> peek() const {
        if (min_heap.empty()) {
            return nullopt;
        } else {
            return min_heap.top();
        }
    }

    optional<T> get(size_t index) const {
        if (index >= array_size && index >= min_heap.size()) {
            return nullopt;
        } else {
            return *(items[index]); // dereference the pointer
        }
    }

    size_t size() const {
        return min_heap.size();
    }

    bool empty() const {
        return min_heap.size() == 0;
    }

    void update(handle_type handle) {
        min_heap.update(handle);
    }

    friend std::ostream& operator <<(std::ostream& os, const RecentWindowHeap<T, Comparator, heap_type, handle_type, MaxSize>& arrayHeap) {
        // print as [1, 2, 3, 4]
        os << "[";
        for (size_t i = 0; i < arrayHeap.array_size; i++) {
            os << arrayHeap.items[i];
            if (i < arrayHeap.array_size - 1) {
                os << ", ";
            }
        }
        if (!arrayHeap.min_heap.empty()) {
            os << "<";
            // print the first element of the min heap
            os << arrayHeap.min_heap.top();
            os << "~>";
        }
        os << "]";
        return os;
    }

private:
    std::array<T*, MaxSize> items;
    size_t array_size = 0;
    size_t round_robin_index = 0;
    heap_type min_heap;
    mutex mtx;
};