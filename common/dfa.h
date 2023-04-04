#pragma once

#include <map>
#include "serialization.h"

template<typename T>
using transition_table = std::map<T, std::map<char, T>>;

// NOTE: this method currently only works well for node labels with a character width <= 4
template <typename T>
std::ostream& operator<<(std::ostream& os, const transition_table<T>& table) {
    for (std::pair<T, std::map<char, T>> tableRow : table) {
        std::cout << tableRow.first << " |\t";
        for (std::pair<char, T> tableCell : tableRow.second) {
            std::cout << tableCell.second << "\t";
        }
        std::cout << std::endl;
    }
    return os;
}

class DFA {
    private:
        std::vector<char> alphabet;
        std::map<int, StateInfo> states;
        transition_table<int> table;

        state_set getForwardConnected(int state);
        state_set getBackwardConnected(int state);

    public:
        DFA(std::vector<char> alphabet, std::map<int, StateInfo> states, transition_table<int> table);
        void optimize();
        void normalize();
        std::pair<bool, int> match(std::string str);
        bool isMatch(std::string str);
        int transition(int s, char c);
        void mergeStates();
        void pruneStates();
        bool isAccepting(int s);
        
        // I'm not a huge fan of the output format since it doesn't include alphabet info
        // This should only be used as an output function
        std::string formatTableForAssignmentOutput();
        static DFA readTableFromAssignmentOutput(std::string tablePath, std::vector<char> alphabet);

        friend std::ostream& operator<<(std::ostream& os, const DFA& table);
};
