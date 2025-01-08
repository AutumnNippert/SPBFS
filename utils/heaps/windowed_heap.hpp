#pragma once

#include <iostream>
#include <list>
#include <vector>
#include <queue>
#include <stdexcept>
#include <optional>
#include <cassert>

using std::optional;
using std::nullopt;

/**
 * Interface for windowed heap data structures.
 */
template <typename T>
class WindowedHeap {
public:
    virtual void push(T item) = 0;
    virtual optional<T> pop() = 0;
    virtual optional<T> peek() const = 0;
    virtual optional<T> get(size_t index) const = 0;
    virtual size_t size() const = 0;
    virtual bool empty() const = 0;
};