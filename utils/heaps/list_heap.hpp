#include "custom_heap.hpp"

template <typename T>
class ListHeap: public CustomHeap<T> {
public:
    ListHeap(size_t max_size) : max_size(max_size) {
        assert (max_size > 0);
    }
    ListHeap(): ListHeap(32) {}
    ~ListHeap() {
        items.clear();
    }

    void push(T item) override {
        if (items.size() >= max_size) { // If the heap is full
            T last_item = items.back();
            items.pop_back();
            min_heap.push(last_item);
        }
        items.push_front(item);
    }

    optional<T> pop() override {
        if (items.empty()) {
            return nullopt;
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

    optional<T> peek() const override {
        if (items.empty()) {
            return nullopt;
        } else {
            return items.front();
        }
    }

    optional<T> get(size_t index) const override {
        if (index >= items.size()) {
            return nullopt;
        } else {
            return *std::next(items.begin(), index);
        }
    }

    size_t size() const override {
        return items.size() + min_heap.size();
    }

    bool empty() const override {
        return items.empty();
    }

    friend std::ostream& operator <<(std::ostream& os, const ListHeap<T>& heap) {
        os << "ListHeap: [";
        for (const T& item : heap.items) {
            os << item << " ";
        }
        os << "]";
        return os;
    }

private:
    size_t max_size;
    std::list<T> items;
    std::priority_queue<T, std::vector<T>, std::greater<T>> min_heap;
};