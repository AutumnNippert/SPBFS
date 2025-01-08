#include <iostream>
#include <list>
#include <vector>
#include <stdexcept>

template <typename T>
class ListHeap {
public:
    ListHeap(size_t max_size) : max_size(max_size) {}

    void push(T item) {
        if (items.size() >= max_size) { // If the heap is full
            T last_item = items.back();
            items.pop_back();
            min_heap.push(last_item);
        }
        items.push_front(item);
    }

    T pop() {
        if (items.empty()) {
            throw std::runtime_error("ListHeap is empty");
        } else{
            T item = items.front();
            items.pop_front();
            if (!min_heap.empty()) { // If there are items in the min heap
                T heapItem = min_heap.top();
                min_heap.pop();
                items.push_back(heapItem);
            }
            return item; // Return the item that was popped
        }
    }

    T peek() const {
        if (items.empty()) {
            throw std::runtime_error("ListHeap is empty");
        } else {
            return items.front();
        }
    }

    size_t size() const {
        return items.size();
    }

    bool empty() const {
        return items.empty();
    }

private:
    size_t max_size;
    std::list<T> items;
    std::priority_queue<T, std::vector<T>, std::greater<T>> min_heap;
};