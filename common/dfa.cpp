#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "dfa.h"
#include "serialization.h"

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
            if (this->states[state].accepting) this->states[keepState].accepting = true;
            if (this->states[state].start) this->states[keepState].start = true;
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

    // swap positions of 0 and starting state to match convention
    int startingId;
    for (auto s : this->states) {
        if (s.second.start) {
            startingId = s.first;
            break;
        }
    }
    if (startingId != 0) {
        int zeroMapped = idMap[0];
        idMap[0] = startingId;
        idMap[startingId] = zeroMapped;
    }

    for (auto tableRow : this->table) {
        for (auto tableCell : tableRow.second) {
            int mappedNodeId;
            // if the table no longer has the node in question, set the mappedNodeId to NC
            if (idMap.find(tableCell.second) == idMap.end()) {
                mappedNodeId = -1;
            }
            else {
                mappedNodeId = idMap[tableCell.second];
            }
            normalized[idMap[tableRow.first]][tableCell.first] = mappedNodeId;
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

int DFA::transition(int state, char c) {
    std::map<char, int> tableRow = this->table[state];
    int nextState = tableRow[c];

    return nextState;
}
bool DFA::isAccepting(int s) {
    return this->states[s].accepting;
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
DFA DFA::readTableFromAssignmentOutput(std::string tablePath, std::vector<char> alphabet) {
    std::ifstream tableFile(tablePath);
    if (!tableFile.good()) {
        std::cerr << "ERROR: could not access table definition file \"" << tablePath << "\"" << std::endl;
        throw 1;
    }

    std::string line;
    char accepting;
    int identifier;
    std::map<int, StateInfo> states;
    transition_table<int> table;

    while (std::getline(tableFile, line)) {
        std::istringstream iss(line);
        iss >> accepting >> identifier;

        StateInfo info;
        info.accepting = accepting == '+';
        info.start = identifier == 0;
        states[identifier] = info;
        
        std::string currentEdge;
        for (int i=0; i<alphabet.size(); i++) {
            iss >> currentEdge;
            int parsedEdge;
            if (currentEdge == "E") parsedEdge = -1;
            else {
                parsedEdge = std::stoi(currentEdge);
            }
            table[identifier][alphabet[i]] = parsedEdge;
        }

    }

    tableFile.close();

    DFA dfa(alphabet, states, table);
    return dfa;
}


std::ostream& operator<<(std::ostream& os, const DFA& dfa) {
    os << "SA\t";
    for (char c : dfa.alphabet) {
        os << c << "\t";
    }
    os << std::endl;

    for (std::pair<int, std::map<char, int>> tableRow : dfa.table) {
        StateInfo info = dfa.states.at(tableRow.first);
        std::cout << (info.start ? '+' : '-') << (info.accepting ? '+' : '-') << " " << tableRow.first << " |\t";
        for (auto tableCell : tableRow.second) {
            std::cout << tableCell.second << "\t";
        }
        std::cout << std::endl;
    }

    return os;
}
