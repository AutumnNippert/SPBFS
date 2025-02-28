#pragma once
#include <boost/container_hash/hash.hpp>
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>

#include "problem_instance.hpp"

using namespace std;



namespace SlidingPuzzle {

    constexpr std::pair<int, int> pos2coord(const char& position){
        int y = position / 4;
        int x = position % 4;
        return std::make_pair(x, y);
    }

    constexpr std::size_t manhattan_distance(const std::pair<int, int>& a, const std::pair<int, int>& b){
        return std::abs(a.first - b.first) + std::abs(a.second - b.second);
    }



    class State {
        private:
        std::uint64_t packed_state;

        inline std::array<char, 16> expand() const{
            std::uint64_t mask = 15;
            std::array<char, 16> retval;
            for(int i = 0; i < 16; i++){
                retval[i] = mask & (packed_state >> 4*i);
            }
            return retval;
        } 

        public:
        
        State() = default;

        State(const std::array<char, 16>& state_array):packed_state(0){
            for(std::uint64_t i = 0; i < 16; i++){
                std::uint64_t val = (std::uint64_t)state_array[i] << (4*i);
                packed_state |= val;
            }
        }

        bool operator == (const State& other) const {
            return packed_state == other.packed_state;
        }

        // Required for use as key in unordered_map
        inline std::size_t hash() const{
            std::size_t seed = 0;
            boost::hash_combine(seed, packed_state);
            return seed;
        }

        string toString() const {
            stringstream ss;

            for (int val : expand()) {
                ss << val << ",";
            }
            return ss.str();
        }

        inline void getSuccessors(vector<State>& successors) const {
            successors.clear();
            auto s = expand();
            auto gap = pos2coord(s[0]);
            for(int i = 1; i < 16; i++){
                auto loc = pos2coord(s[i]);
                if(manhattan_distance(loc, gap) == 1){
                    // is a neighbor
                    auto pg = s[0];
                    auto pos = s[i];
                    //make the change
                    s[i] = pg;
                    s[0] = pos;
                    // push to successors
                    successors.emplace_back(s);
                    // revert change
                    s[i] = pos;
                    s[0] = pg;
                }
            }    
        }
        
        constexpr std::size_t heuristic() const {
            std::size_t h = 0;
            auto s = expand();
            for(char i = 1; i<16; i++){
                auto current = pos2coord(s[i]);
                auto should_be = pos2coord(i);
                h += manhattan_distance(current, should_be);
            } 
            return h;
        }


        friend inline std::ostream& operator << (std::ostream& os, const State& state) {
            os << state.toString();
            return os;
        }
    };



    template<typename State, typename Cost = float>
    class SlidingTileInstance: public ProblemInstance<State, Cost> {
    public:
        SlidingTileInstance(const State& initial, const State& goal) : ProblemInstance<State, Cost>(initial), goal(goal) {}

        static SlidingTileInstance parseInput(std::istream& input) {
            std::array<char, 16> initial_s;
            std::array<char, 16> goal_s;
            int width, height;
            std::string s;
            input >> width >> height; // not sure about order
            assert(width == 4 && height == 4);
            // starting positions for each tile:
            input >> s >> s >> s >> s >> s;
            for(int i = 0; i < 16; i++){
                int x;
                input >> x;
                initial_s[i] = x; 
            }
            // goal positions:
            input >> s >> s;
            for(int i = 0; i < 16; i++){
                int x;
                input >> x;
                goal_s[i] = x; 
            }
            State initial(initial_s);
            State goal(goal_s);
            return SlidingTileInstance<State, Cost>(initial, goal);
        }
        
        // The functions required by ProblemInstance

        constexpr Cost heuristic(const State& state) const override{
            return state.heuristic();
        }

        inline void getSuccessors(const State& state, vector<State>& successors) const override {
            state.getSuccessors(successors);
        }

        Cost getCost(const State&, const State&) const override {
            return 1;  // Each move costs 1
        }

        size_t hash(const State& state) const override {
            return state.hash();
        }

        inline size_t maxActionCount() const override {
            return 4;
        }

        friend std::ostream& operator<<(std::ostream &os, const SlidingTileInstance &st){
            os << "Start: " << st.initial_state << " Goal: " << st.goal;
            return os; 
        }

    private:
        State goal;
    };
}