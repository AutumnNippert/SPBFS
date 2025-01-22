#pragma once

#include <vector>
#include <optional>
#include <ostream>

template <typename T>
class ImmutableCircularQueue {
public:
    ImmutableCircularQueue() = default;
    
    explicit ImmutableCircularQueue(size_t maxSize) : max_size(maxSize) {
        items.resize(max_size);
    }

    ~ImmutableCircularQueue() = default;

    // Returns a new CircularQueue with the item added
    ImmutableCircularQueue push(T item) const {
        if (max_size == 0) return *this;
        ImmutableCircularQueue newQueue = *this; // Copy current state

        newQueue.items[newQueue.round_robin_index] = item;
        newQueue.round_robin_index = (newQueue.round_robin_index + 1) % max_size;
        
        if (newQueue.curr_size < max_size) {
            newQueue.curr_size++;
        }

        return newQueue;
    }

    // Returns the last element added to the queue
    std::optional<T> top() const {
        if (curr_size == 0) {
            return std::nullopt;
        }
        size_t index = round_robin_index == 0 ? max_size - 1 : round_robin_index - 1;
        return items[index];
    }

    // Returns the element at the given index
    std::optional<T> get(size_t index) const {
        return index < curr_size ? std::optional<T>(items[index]) : std::nullopt;
    }

    // Returns the size of the queue
    size_t size() const {
        return curr_size;
    }

    // Checks if the queue is empty
    bool empty() const {
        return curr_size == 0;
    }

    // Overload for printing the queue
    friend std::ostream& operator<<(std::ostream& os, const ImmutableCircularQueue<T>& queue) {
        os << "[";
        for (size_t i = 0; i < queue.curr_size; i++) {
            os << queue.items[i];
            if (i < queue.curr_size - 1) {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }

private:
    std::vector<T> items;
    size_t max_size;
    size_t curr_size = 0;
    size_t round_robin_index = 0;
};
