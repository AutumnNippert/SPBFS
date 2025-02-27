CXX = g++
LIBRARIES = -lboost_graph -libatomic
CXXFLAGS = -std=c++23 -Wall -Wextra -O3 -g -I ./problems -I ./algorithms -I ./utils -I ./utils/heaps -I . -Wno-unused-parameter

# Output binary
TARGET = main

# Source files
SRCS = main.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Test files
TESTS = $(wildcard tests/*.cpp)
TEST_EXECUTABLES = $(TESTS:.cpp=)

# Main rule
$(TARGET): clean $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

# Rule for object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(LIBRARIES) -c $< -o $@

# Rule for test executables
tests: $(TEST_EXECUTABLES)

# Compile each test file into its own executable
tests/%: tests/%.cpp
	$(CXX) $(CXXFLAGS) $(LIBRARIES) $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET) $(TEST_EXECUTABLES)

.PHONY: clean tests
