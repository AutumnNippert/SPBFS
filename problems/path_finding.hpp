#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>

#include "search.hpp"
#include <cmath>

using namespace std;

namespace Pathfinding {
    // static constexpr int SIZE = 5;  // 5x5 grid
    static constexpr char GOAL = '*'; // Represents an empty cell
    static constexpr char WALL = '#'; // Represents a wall
    static constexpr char EMPTY_TILE = '_'; // Represents an empty tile
    static constexpr char ACTOR = 'V'; // Represents the actor

    size_t dimr = 0;
    size_t dimc = 0;

    struct Position {
        int row, col;

        bool operator==(const Position& other) const {
            return row == other.row && col == other.col;
        }
    };

    struct State {
        vector<vector<char>> grid;
        Position actor; // current position of the actor
        vector<Position> goals;
        vector<Position> walls;

        bool operator==(const State& other) const {
            return grid == other.grid && actor == other.actor;
        }

        string toString() const {
            stringstream ss;
            ss << "Actor position: " << actor.row << ", " << actor.col << endl;
            for (const auto& row : grid) {
                for (char val : row) {
                    ss << val << " ";
                }
                ss << "\n";
            }
            return ss.str();
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const State& state) {
        for (const auto& row : state.grid) {
            for (char val : row) {
                os << val << " ";
            }
            os << "\n";
        }
        return os;
    }

    inline vector<Position> getValidMoves(const State& state) {
        vector<Position> moves;
        int row = state.actor.row;
        int col = state.actor.col;

        if (row > 0 && state.grid[row - 1][col] != WALL) moves.push_back({row - 1, col}); // Up
        if (row < dimr - 1 && state.grid[row + 1][col] != WALL) moves.push_back({row + 1, col}); // Down
        if (col > 0 && state.grid[row][col - 1] != WALL) moves.push_back({row, col - 1}); // Left
        if (col < dimc - 1 && state.grid[row][col + 1] != WALL) moves.push_back({row, col + 1}); // Right

        return moves;
    }

    inline void applyMove(State& state, Position move) {
        state.grid[state.actor.row][state.actor.col] = EMPTY_TILE;
        state.actor = move;
        if (state.grid[move.row][move.col] == GOAL) {
            state.grid[move.row][move.col] = EMPTY_TILE;
            // remove it from the state's goals
            state.goals.erase(
                remove(state.goals.begin(), state.goals.end(), move),
                state.goals.end()
            );
        }
        state.grid[move.row][move.col] = ACTOR;
    }

    class PathfindingSolver {
    public:
        static pair<State, State> parseInput(std::istream& input) {
            State state;
            string line;
            
            input >> dimr >> dimc;
            getline(input, line);
            state.grid.resize(dimr, vector<char>(dimc));
            getline(input, line);

            // Read the grid
            for (char i = 0; i < dimr; i++) {
                for (char j = 0; j < dimc; j++) {
                    input >> state.grid[i][j];
                    if (state.grid[i][j] == GOAL) {
                        state.goals.push_back({i, j});
                    } else if (state.grid[i][j] == WALL) {
                        state.walls.push_back({i, j});
                    } else if (state.grid[i][j] == ACTOR) {
                        state.actor = {i, j};
                    } else{
                        continue;
                    }
                }
            }
            return {state, state}; // oof
        }

        static double euclideanDistance(const State& current, const State& goal) {
            double distance = 0;
            for (const auto& goalPos : current.goals) {
                double dx = goalPos.row - current.actor.row;
                double dy = goalPos.col - current.actor.col;
                distance += sqrt(dx * dx + dy * dy);
            }
            return distance;
        }

        static double furthestDistance(const State& current, const State& goal) {
            // get furthest manhattan distance
            double distance = 0;
            for (const auto& goalPos : current.goals) {
                double dx = abs(goalPos.row - current.actor.row);
                double dy = abs(goalPos.col - current.actor.col);
                distance = max(distance, dx + dy);
            }
            return distance;
        }

        static double countGoals(const State& state, const State& goal) {
            return state.goals.size();
        }

        static bool isGoal(const State& state, const State& goal) {
            return state.goals.empty();
        }

        static vector<State> getSuccessors(const State& state) {
            vector<State> successors;
            for (const auto& move : getValidMoves(state)) {
                State newState = state;
                applyMove(newState, move);
                successors.push_back(newState);
            }
            return successors;
        }

        static float getCost(const State&, const State&) {
            return 1.0f; // Each move costs 1
        }

        static size_t hash(const State& state) {
            size_t seed = 0;
            for (const auto& row : state.grid) {
                for (char val : row) {
                    seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                }
            }
            return seed;
        }
    };
}