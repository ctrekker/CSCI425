#include <iostream>
#include <sstream>
#include "dfa.h"
#include "common.h"

DFA::DFA(std::vector<char> alphabet, std::map<int, StateInfo> states, transition_table<int> table) {
    this->alphabet = alphabet;
    this->states = states;
    this->table = table;
}

void DFA::optimize() {
    int tableSize = this->states.size();
    int i = 0;
    while(i < 50) {
        this->mergeStates();
        int newTableSize = this->states.size();
        if(newTableSize < tableSize) {
            tableSize = newTableSize;
        }
        else {
            break;
        }
        i++;
    }

    this->pruneStates();
}

void DFA::mergeStates() {
    std::set<state_set> merges;  // M in pseudocode
    std::vector<std::pair<state_set, std::vector<char>>> heads;  // L in pseudocode

    state_set accepting;
    state_set notAccepting;
    for (std::pair<int, StateInfo> info : this->states) {
        int state = info.first;
        if(info.second.accepting) accepting.insert(state);
        else notAccepting.insert(state);
    }

    heads.push_back(std::make_pair(accepting, this->alphabet));
    heads.push_back(std::make_pair(notAccepting, this->alphabet));

    while (heads.size() > 0) {
        auto SC = heads.back();
        heads.pop_back();
        state_set states = SC.first;
        std::vector<char> alphabet = SC.second;

        char c = alphabet.back();
        alphabet.pop_back();

        std::map<int, state_set> partitions;
        for (int s : states) {
            partitions[this->table[s][c]].insert(s);
        }
        for (std::pair<int, state_set> partitionPair : partitions) {
            state_set partition = partitionPair.second;
            if (partition.size() > 1) {
                if (alphabet.size() == 0) {
                    merges.insert(partition);
                }
                else {
                    heads.push_back(std::make_pair(partition, alphabet));
                }
            }
        }
    }

    transition_table<int> optimized = this->table;

    std::map<int, int> mergeIdMap;
    // populate mergeIdMap as identity to start
    mergeIdMap[-1] = -1;  // NC to NC
    for (std::pair<int, StateInfo> infoPair : this->states) {
        mergeIdMap[infoPair.first] = infoPair.first;
    }
    for (state_set m : merges) {
        int keepState;
        bool first = true;
        for(auto itr = m.begin(); itr != m.end(); itr++) {
            int state = *itr;
            if (first) {
                keepState = state;
                first = false;
                continue;
            }
            optimized.erase(state);
            this->states.erase(state);
            mergeIdMap[state] = keepState;
        }
    }
    // full table update with transitions remapped to keeper nodes
    for (auto tableRow : optimized) {
        for (auto tableCell : tableRow.second) {
            optimized[tableRow.first][tableCell.first] = mergeIdMap[tableCell.second];
        }
    }

    this->table = optimized;

    this->normalize();
}

void DFA::pruneStates() {
    // we conduct pruning by a backwards and forwards pass on the DAG and taking the intersection
    // this process guarentees that there exists a path from the start and accept states
    state_set forwardPass;
    state_set backwardPass;

    std::vector<int> forwardStack;
    std::vector<int> backwardStack;

    for (std::pair<int, StateInfo> info : this->states) {
        int state = info.first;
        if(info.second.accepting) backwardStack.push_back(state);
        if(info.second.start) forwardStack.push_back(state);
    }

    while (forwardStack.size() > 0) {
        int node = forwardStack.back();
        forwardStack.pop_back();
        forwardPass.insert(node);

        for (int connectedState : this->getForwardConnected(node)) {
            if (forwardPass.find(connectedState) == forwardPass.end()) {
                forwardStack.push_back(connectedState);
            }
        }
    }

    while (backwardStack.size() > 0) {
        int node = backwardStack.back();
        backwardStack.pop_back();
        backwardPass.insert(node);

        for (int connectedState : this->getBackwardConnected(node)) {
            if (backwardPass.find(connectedState) == backwardPass.end()) {
                backwardStack.push_back(connectedState);
            }
        }
    }

    state_set activeNodes = stateIntersect(forwardPass, backwardPass);
    for (std::pair<int, StateInfo> info : this->states) {
        int state = info.first;
        
        // node is dead or unreachable
        if (activeNodes.find(state) == activeNodes.end()) {
            this->table.erase(state);
            this->states.erase(state);
        }
    }

    this->normalize();
}

void DFA::normalize() {
    transition_table<int> normalized;
    
    // construct node identifier map
    std::map<int, int> idMap;
    idMap[-1] = -1;
    int i = 0;
    for (auto tableRow : this->table) {
        idMap[tableRow.first] = i;
        i++;
    }

    for (auto tableRow : this->table) {
        for (auto tableCell : tableRow.second) {
            normalized[idMap[tableRow.first]][tableCell.first] = idMap[tableCell.second];
        }
    }

    // normalize the state map
    std::map<int, StateInfo> normalizedStates;
    for (auto infoPair : this->states) {
        normalizedStates[idMap[infoPair.first]] = infoPair.second;
    }

    this->table = normalized;
    this->states = normalizedStates;
}

std::pair<bool, int> DFA::match(std::string str) {
    int state = 0;  // zero is always the starting point by convention
    for (int pos = 0; pos < str.length(); pos++) {
        std::map<char, int> tableRow = this->table[state];
        int nextState = tableRow[str.at(pos)];
        if (nextState == -1) return std::make_pair(false, pos + 1);
        state = nextState;
    }

    bool acc = this->states[state].accepting;
    int accPos = str.length() + 1;
    // account for weird special case in grader for zero-length strings
    if(!acc && str.length() == 0) {
        accPos = 0;
    }
    return std::make_pair(acc, accPos);
}

bool DFA::isMatch(std::string str) {
    return this->match(str).first;
}

state_set DFA::getForwardConnected(int state) {
    state_set conn;
    for (auto tableCell : this->table[state]) {
        if (tableCell.second != -1) {
            conn.insert(tableCell.second);
        }
    }
    return conn;
}
state_set DFA::getBackwardConnected(int state) {
    state_set conn;
    for (auto tableRow : this->table) {
        for (auto tableCell : tableRow.second) {
            if (tableCell.second == state) {
                conn.insert(tableRow.first);
                break;
            }
        }
    }
    return conn;
}


std::string DFA::formatTableForAssignmentOutput() {
    std::stringstream ss;

    int i = 0;
    for (auto tableRow : this->table) {
        ss << (this->states[tableRow.first].accepting ? '+' : '-') << " " << tableRow.first << " ";
        int j = 0;
        for (auto tableCell : tableRow.second) {
            int edge = tableCell.second;
            if (edge == -1) {
                ss << "E";
            }
            else {
                ss << edge;
            }
            j++;
            if (j < this->alphabet.size())
                ss << " ";
        }
        i++;
        if (i < this->table.size())
            ss << std::endl;
    }

    return ss.str();
}


std::ostream& operator<<(std::ostream& os, const DFA& dfa) {
    os << "\t";
    for (char c : dfa.alphabet) {
        os << c << "\t";
    }
    os << std::endl;

    os << dfa.table;

    return os;
}
