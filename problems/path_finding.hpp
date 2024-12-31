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
    static constexpr int SIZE = 5;  // 5x5 grid
    static constexpr char GOAL = '*'; // Represents an empty cell
    static constexpr char WALL = '#'; // Represents a wall
    static constexpr char EMPTY_TILE = '_'; // Represents an empty tile
    static constexpr char ACTOR = '@'; // Represents the actor

    struct Position {
        int x, y;

        bool operator==(const Position& other) const {
            return x == other.x && y == other.y;
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
            ss << "Actor position: " << actor.x << ", " << actor.y << endl;
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
        int x = state.actor.x;
        int y = state.actor.y;

        if (x > 0 && state.grid[x - 1][y] != WALL) moves.push_back({x - 1, y}); // Up
        if (x < SIZE - 1 && state.grid[x + 1][y] != WALL) moves.push_back({x + 1, y}); // Down
        if (y > 0 && state.grid[x][y - 1] != WALL) moves.push_back({x, y - 1}); // Left
        if (y < SIZE - 1 && state.grid[x][y + 1] != WALL) moves.push_back({x, y + 1}); // Right

        return moves;
    }

    inline void applyMove(State& state, Position move) {
        state.grid[state.actor.x][state.actor.y] = EMPTY_TILE;
        state.actor = move;
        if (state.grid[move.x][move.y] == GOAL) {
            state.grid[move.x][move.y] = EMPTY_TILE;
            // remove it from the state's goals
            state.goals.erase(
                remove(state.goals.begin(), state.goals.end(), move),
                state.goals.end()
            );
        }
        state.grid[move.x][move.y] = ACTOR;
    }

    class PathfindingSolver {
    public:
        static pair<State, State> parseInput(std::istream& input) {
            State state;
            state.grid.resize(SIZE, vector<char>(SIZE));

            // Read the grid
            for (char i = 0; i < SIZE; i++) {
                for (char j = 0; j < SIZE; j++) {
                    input >> state.grid[i][j];
                    if (state.grid[i][j] == EMPTY_TILE) {
                        continue;
                    } else if (state.grid[i][j] == GOAL) {
                        state.goals.push_back({i, j});
                    } else if (state.grid[i][j] == WALL) {
                        state.walls.push_back({i, j});
                    } else if (state.grid[i][j] == ACTOR) {
                        state.actor = {i, j};
                    }
                }
            }
            return {state, state}; // oof
        }

        static double euclideanDistance(const State& current, const State& goal) {
            double distance = 0;
            for (const auto& goalPos : current.goals) {
                double dx = goalPos.x - current.actor.x;
                double dy = goalPos.y - current.actor.y;
                distance += sqrt(dx * dx + dy * dy);
            }
            return distance;
        }

        static double furthestDistance(const State& current, const State& goal) {
            // get furthest manhattan distance
            double distance = 0;
            for (const auto& goalPos : current.goals) {
                double dx = abs(goalPos.x - current.actor.x);
                double dy = abs(goalPos.y - current.actor.y);
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