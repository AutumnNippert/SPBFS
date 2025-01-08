#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <algorithm>  // For std::sort
#include <string>
#include "array_heap.hpp"

using namespace std;

void testArrayHeap() {
    ArrayHeap<int> heap(5); // Create a heap with a maximum size of 5

    // Test pushing elements
    cout << "Pushing elements into the heap:" << endl;
    for (int i = 1; i <= 8; ++i) {
        heap.push(i);
        cout << "Pushed: " << i << endl;
    }
    cout << heap << endl;

    // remove 3
    heap.pop();
    heap.pop();
    heap.pop();
    cout << heap << endl;

    // Test peeking the top element
    auto top = heap.peek();
    if (top) {
        cout << "Top element (peek): " << *top << endl;
    }

    // Test popping elements
    cout << "Popping elements from the heap:" << endl;
    while (!heap.empty()) {
        auto popped = heap.pop();
        if (popped) {
            cout << "Popped: " << *popped << endl;
        }
    }
    cout << heap << endl;

    // Test popping from an empty heap
    auto emptyPop = heap.pop();
    if (!emptyPop) {
        cout << "Heap is empty, nothing to pop." << endl;
    }
    cout << heap << endl;

    // try get 2
    auto get2 = heap.get(2);
    if(!get2) {
        cout << "Index 2 is out of bounds" << endl;
    } else {
        cout << "Index 2: " << *get2 << endl;
    }

    // add 3 things
    heap.push(1);
    heap.push(2);
    heap.push(3);

    // try get 2
    get2 = heap.get(2);
    if(!get2) {
        cout << "Index 2 is out of bounds" << endl;
    } else {
        cout << "Index 2: " << *get2 << endl;
    }

    cout << heap << endl;
}
// Helper function: print a test message, pass/fail
void printTestResult(const std::string& testName, bool condition) {
    std::cout << (condition ? "[PASS] " : "[FAIL] ") << testName << "\n";
}

// Test 1: Basic functionality with small array
void testBasic() {
    std::cout << "\n=== TEST 1: Basic small-scale push/pop ===\n";

    ArrayHeap<int> heap(3);  // capacity = 3
    printTestResult("Initial heap should be empty", heap.empty());

    // Push 1, 2, 3
    heap.push(1);
    heap.push(2);
    heap.push(3);

    printTestResult("Heap size after pushing 3 items == 3", heap.size() == 3);
    printTestResult("Heap should not be empty now", !heap.empty());

    std::cout << "Heap content: " << heap << "\n";

    // Push 4th item, should cause overflow into min_heap
    heap.push(99);

    std::cout << "Heap content after pushing 4th (overflow): " << heap << "\n";
    std::cout << "size(): " << heap.size() << " (front array + min_heap)\n";

    // Pop 1 item
    auto popped = heap.pop();
    if (popped.has_value()) {
        std::cout << "Popped: " << popped.value() << "\n";
    } else {
        std::cout << "Popped: nullopt\n";
    }
    std::cout << "Heap content after 1 pop: " << heap << "\n";
}

// Test 2: Pop until empty
void testPopUntilEmpty() {
    std::cout << "\n=== TEST 2: Pop until empty ===\n";

    ArrayHeap<int> heap(5);
    for (int i = 0; i < 5; i++) {
        heap.push(i);
    }
    // Now push a couple more, which go into min_heap
    heap.push(100);
    heap.push(200);

    std::cout << "Heap content before popping all: " << heap << "\n";
    std::cout << "size(): " << heap.size() << "\n";

    // Keep popping until empty
    int popCount = 0;
    while (!heap.empty()) {
        auto val = heap.pop();
        popCount++;
        // Just show the popped value
        if (val.has_value()) {
            std::cout << "Popped " << val.value() << " | ";
        }
    }
    std::cout << "\nPopped " << popCount << " items total.\n";
    printTestResult("Heap should be empty", heap.empty());
}

// Test 3: Random stress test
void testRandomLarge(size_t nInserts, size_t capacity) {
    std::cout << "\n=== TEST 3: Random large-scale test ===\n";
    std::cout << "Inserting " << nInserts << " random integers into ArrayHeap(capacity=" 
              << capacity << ").\n";

    ArrayHeap<int> heap(capacity);

    // Use a random engine
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100000);

    // We'll push random values and also keep a reference vector for verification if needed
    std::vector<int> reference;

    // Insert random integers
    for (size_t i = 0; i < nInserts; i++) {
        int val = dist(gen);
        heap.push(val);
        reference.push_back(val);
    }

    // We won't do a direct 1:1 comparison with a standard data structure here,
    // but you could if you want. We'll just check the final size for sanity:
    bool sizeCheck = (heap.size() == reference.size());
    std::cout << "Final reported heap.size() = " << heap.size() 
              << ", actual inserted = " << reference.size() << "\n";
    printTestResult("Heap size matches number inserted (for capacity >= nInserts)", sizeCheck && capacity >= nInserts);

    // Let's do a partial pop test:
    // Pop ~20 items (or until empty) and ensure we don't crash
    int popCount = std::min<size_t>(20, reference.size());
    std::cout << "Popping " << popCount << " items:\n";
    for (int i = 0; i < popCount; i++) {
        auto val = heap.pop();
        if (val.has_value()) {
            std::cout << val.value() << " ";
        } else {
            std::cout << "null ";
        }
    }
    std::cout << "\nsize() after popping ~20: " << heap.size() << "\n";
}

// Test 4: Confirm sorted front portion (if you used the sorting version)
void testSortedFront() {
    std::cout << "\n=== TEST 4: Check if front array is sorted (for the sorted variant) ===\n";
    // This test only makes sense if you specifically used the version that keeps the array sorted.
    ArrayHeap<int> heap(5);

    // Insert a bunch of random numbers
    std::vector<int> values = {10, 3, 25, 1, 7, 12, 2, 30, 11};
    for (int v : values) {
        heap.push(v);
    }

    // Now, if the array portion is sorted ascending, it should read from smallest -> largest.
    // We'll read the portion that belongs to the array, ignoring the min_heap portion for now.
    // The array portion can be at most 5 elements.

    // Extract the array portion
    std::vector<int> front;
    for (size_t i = 0; i < 5; i++) {
        auto val = heap.get(i); 
        if (!val.has_value()) break; 
        front.push_back(val.value());
    }

    // Check if 'front' is sorted ascending
    bool sorted = std::is_sorted(front.begin(), front.end());
    std::cout << "Front portion: [";
    for (size_t i = 0; i < front.size(); i++) {
        std::cout << front[i];
        if (i + 1 < front.size()) std::cout << ", ";
    }
    std::cout << "]\n";

    printTestResult("Front array is sorted ascending", sorted);
}

int main() {
    testArrayHeap();
        // Run all tests
    testBasic();
    testPopUntilEmpty();

    // Large random test:
    //  - 5,000 inserts into capacity=10,000 => no overflow
    //  - Then 5,000 inserts into capacity=1,000 => guaranteed overflow
    testRandomLarge(/*nInserts=*/5000, /*capacity=*/10000);  
    testRandomLarge(/*nInserts=*/5000, /*capacity=*/1000);

    // Check sorted front (only meaningful if you used the "sorted insertion" version).
    testSortedFront();

    std::cout << "\nAll tests completed.\n";
    return 0;
}