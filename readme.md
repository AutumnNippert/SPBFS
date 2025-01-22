# Search
This repo is used for testing the effectivity of search algorithms on different problems.

# Requirements
This repo requires boost [1.86.0](https://www.boost.org/users/history/version_1_86_0.html)

# Search Algorithms
Search Algorithms implement the `Search` class in `./algorithms/search.hpp`. They are templated with `<State, Cost>`. They require the following:
```c++
virtual std::vector<State> findPath() = 0;
```
And have the variables
```c++
size_t expandedNodes = 0; // Number of nodes expanded during the search
size_t generatedNodes = 0; // Number of nodes generated during the search
size_t duplicatedNodes = 0; // Number of nodes duplicated during the search

const ProblemInstance<State, Cost>& problemInstance;
```

# Problems
ProblemInstanceClasses implement `ProblemInstance`, requiring the following:
```c++
virtual std::vector<State> getSuccessors(const State& state) const = 0;
virtual Cost heuristic(const State& state) const = 0;
virtual Cost getCost(const State& state, const State& successor) const = 0;
virtual size_t hash(const State& state) const = 0;
```

It is also helpful to override the `<<` operator as to print the states for easy visualization:
```c++
inline std::ostream& operator << (std::ostream& os, const State& s){
    os << s.toString();
    return os;
}
```

# Running it
```
$ ./main -a <algorithm> -p <problem> -t <threads=1> < <data-file>
```

# Testing:
## Collecting Data
```
$ python3 collect_data.py kbfs path 1-4 1 0-1000 500 sample-data/path_data/testing 4
```

## Creating Plots
```
$ python3 compare_algorithms.py 1-8 1 1000 cafe kbfs
```