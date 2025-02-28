#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iostream>

#include "search.hpp"
#include "problem_instance.hpp"
#include "position.hpp"

#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_set.hpp>

using namespace std;

using boost::unordered_set;
using boost::unordered_flat_map;

namespace Pathfinding {
    static constexpr char GOAL = '*'; // Represents an empty cell
    static constexpr char WALL = '#'; // Represents a wall
    static constexpr char EMPTY_TILE = '_'; // Represents an empty tile
    static constexpr char ACTOR = 'V'; // Represents the actor

    struct State {
        Position actor; // current position of the actor
        boost::unordered_set<Position> goals; // set of goal positions

        bool operator==(const State& other) const {
            return actor == other.actor && goals == other.goals;
        }

        string toString() const {
            stringstream ss;
            ss << "{[Actor position: " << actor.row << ", " << actor.col << "], Goals: [";
            for (const auto& goal : goals) {
                ss << goal.row << ", " << goal.col << "; ";
            }
            ss << "]}";
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
        PathfindingInstance(size_t rows, size_t cols, boost::unordered_set<Position> walls, State& initial_state) : ProblemInstance<State, Cost>(initial_state) {
            this->dimr = rows;
            this->dimc = cols;
            this->walls = walls;
        }

        /**
         * Parse the input for the pathfinding problem and return the instance
         */
        static PathfindingInstance parseInput(std::istream& input) {
            State state;
            string line;
            size_t dimr, dimc;
            boost::unordered_set<Position> walls;
            
            // read dimensions
            input >> dimr >> dimc;
            input.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard the leftover newline
            getline(input, line);

            // Read the grid
            for (size_t i = 0; i < dimr; i++) {
                getline(input, line); // Read a full row of the grid
                // cout << line << endl;
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
            auto instance = PathfindingInstance(dimr, dimc, walls, state);
            return instance;
        }

        // Gets valid moves up, down, left, and right if no obstacles and within bounds
        inline vector<Position> getValidMoves(const State& state) const {
            vector<Position> moves;
            size_t row = state.actor.row;
            size_t col = state.actor.col;
            if (row > 0 && walls.find(Position{row - 1, col}) == walls.end()) moves.push_back({row - 1, col}); // Up
            if (dimr > 1 && row < dimr - 1 && walls.find(Position{row + 1, col}) == walls.end()) moves.push_back({row + 1, col}); // Down
            if (col > 0 && walls.find(Position{row, col - 1}) == walls.end()) moves.push_back({row, col - 1}); // Left
            if (dimc > 1 && col < dimc - 1 && walls.find(Position{row, col + 1}) == walls.end()) moves.push_back({row, col + 1}); // Right

            return moves;
        }

        // The functions required by ProblemInstance

        inline float heuristic(const State& state) const override {
            if (state.goals.empty()) {
                return 0.0f; // No goals left, heuristic cost is zero
            }
            // get the closest goal and return the distance
            float minDist = std::numeric_limits<float>::max();
            for (const auto& goal : state.goals) {
                float dist = std::abs((int)goal.row - (int)state.actor.row) + std::abs((int)goal.col - (int)state.actor.col); // i don't want to cast to an int but size_t subtraction sucks
                if (dist < minDist) {
                    minDist = dist;
                }
            }
            return minDist;
        }

        inline void getSuccessors(const State& state, vector<State>& successors) const override {
            successors.clear();
            for (const auto& move : this->getValidMoves(state)) {
                State newState = state;
                applyMove(newState, move);
                successors.push_back(newState);
            }
        }

        inline float getCost(const State&, const State&) const override {
            return 1.0f; // Each move costs 1
        }

        inline size_t hash(const State& state) const override {
            std::size_t seed = 0;
            boost::hash_combine(seed, state.actor.row);
            boost::hash_combine(seed, state.actor.col);
            boost::hash_combine(seed, state.goals.size());
            return seed;
        }

        inline size_t maxActionCount() const override {
            return 4;
        }

    private:
        size_t dimr = 0; // Number of rows in the grid
        size_t dimc = 0; // Number of columns in the grid
        boost::unordered_set<Position> walls; // Set of wall positions
    };
}