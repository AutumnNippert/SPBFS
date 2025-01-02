#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>

#include "search.hpp"
#include "problem.hpp"
#include <cmath>

#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_set.hpp>

using namespace std;

namespace Pathfinding {
    // static constexpr int SIZE = 5;  // 5x5 grid
    static constexpr char GOAL = '*'; // Represents an empty cell
    static constexpr char WALL = '#'; // Represents a wall
    static constexpr char EMPTY_TILE = '_'; // Represents an empty tile
    static constexpr char ACTOR = 'V'; // Represents the actor

    struct Position {
        size_t row, col;

        bool operator==(const Position& other) const {
            return row == other.row && col == other.col;
        }
    };

    struct PositionHash {
        size_t operator()(const Position& pos) const {
            return pos.row * 1000 + pos.col;
        }
    };

    struct State {
        Position actor; // current position of the actor
        boost::unordered_set<Position, PositionHash> goals;

        bool operator==(const State& other) const {
            return actor == other.actor && goals == other.goals;
        }

        string toString() const {
            stringstream ss;
            ss << "{[Actor position: " << actor.row << ", " << actor.col << "], Goals: [";
            for (const auto& goal : goals) {
                ss << goal.row << ", " << goal.col << "; ";
            }
            ss << "]}" << endl;
            return ss.str();
        }
    };

    inline std::ostream& operator << (std::ostream& os, const State& s){
        os << s.toString();
        return os;
    }

    inline void applyMove(State& state, Position move) {
        state.actor = move;
        if (state.goals.find(move) != state.goals.end()) {
            state.goals.erase(move);
        }
    }

    template<typename State, typename Cost = float>
    class PathfindingInstance: public ProblemInstance<State, Cost> {
    public:
        PathfindingInstance(size_t rows, size_t cols, boost::unordered_set<Position, PositionHash> walls, State initial_state) {
            this->dimr = rows;
            this->dimc = cols;
            this->walls = walls;
            this->initial_state = initial_state;
            cout << "State gotten on construction:" << endl;
            cout << "&" << this << endl;
            cout << initial_state << endl;
        }

        static PathfindingInstance parseInput(std::istream& input) {
            State state;
            string line;
            size_t dimr, dimc;
            boost::unordered_set<Position, PositionHash> walls;
            
            input >> dimr >> dimc;
            input.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard the leftover newline
            getline(input, line);

            // Read the grid
            for (size_t i = 0; i < dimr; i++) {
                getline(input, line); // Read a full row of the grid
                cout << line << endl;
                for (size_t j = 0; j < dimc; j++) {
                    char curr = line[j];
                    if (curr == GOAL) {
                        state.goals.emplace(Position{i, j});
                    } else if (curr == WALL) {
                        walls.emplace(Position{i, j});
                    } else if (curr == ACTOR) {
                        state.actor = {i, j};
                    }
                }
            }
            cout << "State on Parse:" << endl;
            cout << state << endl;
            return PathfindingInstance(dimr, dimc, walls, state);
        }

        float heuristic(const State& state, const State& goal) override {
            size_t dirtCount = state.goals.size();
            cout << "Dirt count: " << dirtCount << endl;
            return dirtCount;
        }

        bool isGoal(const State& state, const State& goal) {
            return state.goals.empty();
        }

        vector<State> getSuccessors(const State& state) override {
            vector<State> successors;
            for (const auto& move : getValidMoves(state)) {
                State newState = state;
                applyMove(newState, move);
                successors.push_back(newState);
            }
            return successors;
        }

        vector<Position> getValidMoves(const State& state) {
            vector<Position> moves;
            size_t row = state.actor.row;
            size_t col = state.actor.col;
            if (row > 0 && walls.find(Position{row - 1, col}) == walls.end()) moves.push_back({row - 1, col}); // Up
            if (dimr > 1 && row < dimr - 1 && walls.find(Position{row + 1, col}) == walls.end()) moves.push_back({row + 1, col}); // Down
            if (col > 0 && walls.find(Position{row, col - 1}) == walls.end()) moves.push_back({row, col - 1}); // Left
            if (dimc > 1 && col < dimc - 1 && walls.find(Position{row, col + 1}) == walls.end()) moves.push_back({row, col + 1}); // Right

            return moves;
        }

        float getCost(const State&, const State&) override {
            return 1.0f; // Each move costs 1
        }

        size_t hash(const State& state) override {
            size_t h = state.actor.row * dimc + state.actor.col;
            for (const auto& goal : state.goals) {
                h += goal.row * dimc + goal.col;
            }
            return h;
        }

    private:
        size_t dimr = 0;
        size_t dimc = 0;
        boost::unordered_set<Position,PositionHash> walls;
        State initial_state;
    };
}