CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -g -I ./problems -I ./algorithms -I .

# Output binary
TARGET = main

# Source files
SRCS = main.cpp

# Header files
HEADERS = search.hpp astar.hpp sliding_puzzle.hpp path_finding.hpp

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
