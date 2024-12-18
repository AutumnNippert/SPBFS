CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra

# Output binary
TARGET = main

# Source files
SRCS = main.cpp

# Header files
HEADERS = search.hpp astar.hpp sliding_puzzle.hpp sliding_puzzle_solver.hpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Main rule
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET)

# Rule for object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
