#include "windowed_heap.hpp"

/**
 * ArrayHeap is a data structure that is a combination of an array and a min heap.
 * The array is used to store the first n elements, and the min heap is used to store the rest.
 * When an element is popped from the array, the first element from the min heap is moved to the array.
 * This allows for fast access to the first n elements, and fast access to the minimum element.
 */

template <typename T>
class ArrayHeap: public WindowedHeap<T> {
public:
    ArrayHeap(size_t max_size) : max_size(max_size), items(new T[max_size]) {
        assert (max_size > 0);
    }
    ArrayHeap(): ArrayHeap(32) {}

    ~ArrayHeap() {
        delete[] items;
    }

    void push(T item) override {
        if (array_size >= max_size) { // If the list is full
            T last_item = items[array_size - 1];
            min_heap.push(last_item);
        }else{
            array_size++;
        }
        move_array_right();
        items[0] = item;
    }

    optional<T> pop() override {
        if (array_size == 0) {
            return nullopt;
        }

        T item = items[0];
        move_array_left();
        if (!min_heap.empty()) { // If there are items in the min heap
            T heapItem = min_heap.top();
            min_heap.pop();
            items[array_size-1] = heapItem;
        } else {
            array_size--;
        }
        assert(array_size <= max_size);
        return item; // Return the item that was popped
    }

    optional<T> peek() const override {
        if (array_size == 0) {
            return nullopt;
        } else {
            return items[0];
        }
    }

    optional<T> get(size_t index) const override {
        if (index >= array_size) {
            return nullopt;
        } else {
            return items[index];
        }
    }



    size_t size() const override {
        return array_size + min_heap.size();
    }

    bool empty() const override {
        return array_size == 0;
    }

    friend std::ostream& operator <<(std::ostream& os, const ArrayHeap<T>& arrayHeap) {
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
    T* items;
    size_t array_size = 0;

    std::priority_queue<T, std::vector<T>, std::greater<T>> min_heap;

    void move_array_left() {
        for (size_t i = 0; i < array_size - 1; i++) {
            items[i] = items[i + 1];
        }
    }

    void move_array_right() {
        for (size_t i = array_size - 1; i > 0; i--) {
            items[i] = items[i - 1];
        }
    }
};