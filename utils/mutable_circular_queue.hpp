#pragma once

#include <array>
#include <optional>
#include <ostream>

template <typename T, size_t MaxSize>
class MutableCircularQueue {
public:
    MutableCircularQueue() {
        static_assert(MaxSize > 0, "MaxSize must be greater than 0");
    }

    ~MutableCircularQueue() = default;

    // Add an item to the queue
    void push(const T& item) {
        items[head_index] = item;  // Add the item at the current head index
        head_index = (head_index + 1) % MaxSize;  // Move the head index in a circular manner

        if (current_size < MaxSize) {
            ++current_size;  // Increment size if it's less than the maximum
        }
    }

    // Get the last element added to the queue (most recent item)
    std::optional<T> top() const {
        if (current_size == 0) {
            return std::nullopt;  // Return empty optional if the queue is empty
        }
        size_t last_index = (head_index == 0) ? MaxSize - 1 : head_index - 1;
        return items[last_index];
    }

    // Get the element at a specific index
    std::optional<T> get(size_t index) const {
        if (index >= current_size) {
            return std::nullopt;  // Return empty optional if index is out of range
        }
        return items[index];
    }

    // Get the current number of elements in the queue
    size_t size() const {
        return current_size;
    }

    // Check if the queue is empty
    bool empty() const {
        return current_size == 0;
    }

    // Overload the stream operator to display the queue contents
    friend std::ostream& operator<<(std::ostream& os, const MutableCircularQueue<T, MaxSize>& queue) {
        os << "[";
        for (size_t i = 0; i < queue.current_size; ++i) {
            os << queue.items[i];
            if (i < queue.current_size - 1) {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }

private:
    std::array<T, MaxSize> items{};  // Fixed-size array to hold the queue items
    size_t current_size = 0;        // Number of elements in the queue
    size_t head_index = 0;          // Index where the next item will be added
};
