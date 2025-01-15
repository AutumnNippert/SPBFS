#pragma once

#include <array>
#include <optional>
#include <ostream>

template <typename T, size_t MaxSize>
class ImmutableCircularQueue {
public:
    ImmutableCircularQueue() {
        static_assert(MaxSize > 0, "MaxSize must be greater than 0");
    }

    ~ImmutableCircularQueue() = default;

    // Returns a new CircularQueue with the item added
    ImmutableCircularQueue push(T item) const {
        ImmutableCircularQueue newQueue = *this; // Copy current state

        // Store the item
        newQueue.items[newQueue.round_robin_index] = item;

        // Increment round_robin_index
        newQueue.round_robin_index++;
        if (newQueue.round_robin_index >= MaxSize) {
            newQueue.round_robin_index = 0;
        }

        // Increment size if it is less than MaxSize
        if (newQueue.curr_size < MaxSize) {
            newQueue.curr_size++;
        }

        return newQueue;
    }

    // Returns the last element added to the queue
    std::optional<T> top() const {
        if (curr_size == 0) {
            return std::nullopt;
        }
        size_t index = round_robin_index == 0 ? MaxSize - 1 : round_robin_index - 1;
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
    friend std::ostream& operator<<(std::ostream& os, const ImmutableCircularQueue<T, MaxSize>& queue) {
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
    std::array<T, MaxSize> items{};
    size_t curr_size = 0;
    size_t round_robin_index = 0;
};
