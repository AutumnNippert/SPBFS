#include "windowed_heap.hpp"

#include <boost/heap/d_ary_heap.hpp>

/**
 * RecentWindowHeap is a heap paired with an array that stores copies of the most recent elements in a round robin fashion.
 */
template <typename T>
class RecentWindowHeap : public WindowedHeap<T> {
public:
    RecentWindowHeap(size_t max_size) : max_size(max_size), items(new T*[max_size]) {
        assert (max_size > 0);
    }
    RecentWindowHeap(): RecentWindowHeap(32) {}

    ~RecentWindowHeap() {
        delete[] items;
    }

    void push(T item) {
        min_heap.push(item);
        items[round_robin_index++] = &item; // store pointer to the item

        // increment array_size if it is less than max_size
        if (array_size < max_size) {
            array_size++;
        }

        // reset round_robin_index
        if (round_robin_index >= max_size) {
            round_robin_index = 0;
        }
    }

    optional<T> pop() override {
        if(min_heap.empty()) {
            return nullopt;
        }
        auto t = min_heap.top();
        min_heap.pop();
        return t;
    }

    optional<T> peek() const override {
        if (min_heap.empty()) {
            return nullopt;
        } else {
            return min_heap.top();
        }
    }

    optional<T> get(size_t index) const override {
        if (index >= array_size && index >= min_heap.size()) {
            return nullopt;
        } else {
            return *(items[index]); // dereference the pointer
        }
    }

    size_t size() const override {
        return min_heap.size();
    }

    bool empty() const override {
        return min_heap.size() == 0;
    }

    friend std::ostream& operator <<(std::ostream& os, const RecentWindowHeap<T>& arrayHeap) {
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
    size_t max_size;
    T** items;
    size_t array_size = 0;
    size_t round_robin_index = 0;
    boost::heap::d_ary_heap<T, boost::heap::arity<2>, boost::heap::mutable_<true>> min_heap;
};