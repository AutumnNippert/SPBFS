#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <algorithm>  // For sort
#include <string>
#include "custom_heap.hpp"
#include "array_heap.hpp"
#include "list_heap.hpp"
#include "recent_window_heap.hpp"

using namespace std;

template <typename HeapType>
bool testHeapImplementation() {
    HeapType heap(5); // Create a heap with a maximum size of 5
    bool passed = true;

    // Test pushing elements
    for (int i = 1; i <= 8; ++i) {
        heap.push(i);
    }

    // Remove 3 elements
    for (int i = 0; i < 3; ++i) {
        heap.pop();
    }

    // Test peeking the top element
    if (heap.peek() == nullopt) {
        cout << "Error: Expected a top element after popping 3 elements.\n";
        passed = false;
    }

    // Test popping elements
    while (!heap.empty()) {
        heap.pop();
    }

    // Test popping from an empty heap
    if (heap.pop() != nullopt) {
        cout << "Error: Expected nullopt when popping from an empty heap.\n";
        passed = false;
    }

    return passed;
}

// Test 1: Basic functionality with small array
template <typename HeapType>
bool testBasic() {
    HeapType heap(3);
    bool passed = true;

    if (!heap.empty()) {
        cout << "Error: Initial heap should be empty.\n";
        passed = false;
    }

    heap.push(1);
    heap.push(2);
    heap.push(3);
    if (heap.size() != 3) {
        cout << "Error: Heap size after pushing 3 items should be 3.\n";
        passed = false;
    }
    if (heap.empty()) {
        cout << "Error: Heap should not be empty now.\n";
        passed = false;
    }

    // Push 4th item, should cause overflow into min_heap
    heap.push(99);
    if (heap.size() != 4) {
        cout << "Error: Heap size after pushing 4th item should be 4.\n";
        passed = false;
    }

    auto popped = heap.pop();
    if (!popped.has_value()) {
        cout << "Error: Expected a value when popping from the heap.\n";
        passed = false;
    }

    return passed;
}

// Test 2: Pop until empty
template <typename HeapType>
bool testPopUntilEmpty() {
    HeapType heap(5);
    bool passed = true;

    for (int i = 0; i < 5; i++) {
        heap.push(i);
    }
    heap.push(100);
    heap.push(200);

    // Keep popping until empty
    while (!heap.empty()) {
        heap.pop();
    }
    if (!heap.empty()) {
        cout << "Error: Heap should be empty after popping all elements.\n";
        passed = false;
    }

    return passed;
}

// Test 3: Random stress test
template <typename HeapType>
bool testRandomLarge(size_t nInserts, size_t capacity) {
    HeapType heap(capacity);
    vector<int> reference;
    bool passed = true;

    // Insert random integers
    for (size_t i = 0; i < nInserts; i++) {
        int val = rand() % 100000 + 1; // Random value
        heap.push(val);
        reference.push_back(val);
    }

    // Check size
    if (heap.size() != reference.size()) {
        cout << "Error: Heap size does not match the number of inserted elements.\n";
        passed = false;
    }

    // Pop ~20 items
    for (int i = 0; i < min<size_t>(20, reference.size()); i++) {
        heap.pop();
    }

    return passed;
}

// Test 4: Confirm sorted front portion
template <typename HeapType>
bool testSortedFront() {
    HeapType heap(5);
    vector<int> values = {10, 3, 25, 1, 7, 12, 2, 30, 11};
    bool passed = true;

    for (int v : values) {
        heap.push(v);
    }

    // Extract the array portion
    vector<int> front;
    for (size_t i = 0; i < 5; i++) {
        auto val = heap.get(i); 
        if (!val.has_value()) break; 
        front.push_back(val.value());
    }

    // Check if 'front' is sorted ascending
    if (!is_sorted(front.begin(), front.end())) {
        cout << "Error: Front array is not sorted ascending.\n";
        passed = false;
    }

    return passed;
}

// test storing pointers
template <typename HeapType>
bool testIntPointers() {
    HeapType heap(5);
    bool passed = true;
    
    vector<int*> pointers;
    for (int i = 0; i < 5; i++) {
        int* val = new int(i);
        heap.push(val);
        pointers.push_back(val);
    }

    // Pop all elements
    while (!heap.empty()) {
        auto val = heap.pop();
        if (!val.has_value()) {
            cout << "Error: Expected a value when popping from the heap.\n";
            passed = false;
        }
        delete val.value();
    }

    return passed;
}

template <typename HeapType, typename Func>
bool runTest(const string& heapName, const string& testName, Func testFunc) {
    bool result = testFunc();
    if (result) {
        cout << "[PASS] (" << heapName << ") " << testName << endl;
    } else {
        cout << "[FAIL] (" << heapName << ") " << testName << endl;
    }
    return result;
}

#define RUN_TEST(HeapType, TestFunc) \
    do {                                                           \
        bool localResult = runTest<HeapType>(                      \
            #HeapType,                                             \
            #TestFunc,                                             \
            [](){ return TestFunc<HeapType>(); }                   \
        );                                                         \
        if (localResult) { ++passed; totalPassed++;} else { ++failed; totalFailed;}          \
    } while(0)

size_t totalPassed = 0, totalFailed = 0;

template <typename HeapType>
void testHeap(const string& heapName, HeapType heap) {
    size_t passed = 0, failed = 0;
    cout << "=== " << heapName << " TEST RESULTS ===" << endl;
    RUN_TEST(HeapType, testHeapImplementation);
    RUN_TEST(HeapType, testBasic);
    RUN_TEST(HeapType, testPopUntilEmpty);
    bool res = runTest<HeapType>(heapName, "testRandomLarge (5000 inserts, 1000 capacity)", [](){
        return testRandomLarge<HeapType>(5000, 1000);
    });
    if (res) {
        ++passed;
    } else {
        ++failed;
    }
    RUN_TEST(HeapType, testSortedFront);

    totalPassed += passed;
    totalFailed += failed;

    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    cout << endl;
}

int main() {
    size_t passed = 0, failed = 0;
    testHeap("ArrayHeap<int>", ArrayHeap<int>());
    testHeap("ListHeap<int>", ListHeap<int>());
    testHeap("RecentWindowHeap<int>", RecentWindowHeap<int>());

    cout << "=== ADDITIONAL TESTS ===" << endl;

    RUN_TEST(ArrayHeap<int*>, testIntPointers);
    RUN_TEST(ListHeap<int*>,  testIntPointers);
    RUN_TEST(RecentWindowHeap<int*>, testIntPointers);

    totalPassed += passed;
    totalFailed += failed;

    cout << "\n=== TESTING COMPLETE ===" << endl;

    // count passed and failed tests
    cout << "Passed: " << totalPassed << endl;
    cout << "Failed: " << totalFailed << endl;

    return 0;
}
