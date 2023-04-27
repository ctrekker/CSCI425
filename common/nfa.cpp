#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <set>

#include "nfa.h"
#include "dfa.h"
#include "lexer.h"

bool isEmpty(std::ifstream& pFile)
{
    return pFile.peek() == std::ifstream::traits_type::eof();
}

Definition readDefinition(std::string filePath, bool skipHead) {
    // init definition structure
    Definition def;
    DefinitionHeader head;
    std::vector<DefinitionLine> lines;

    std::string line;
    std::ifstream defFile(filePath);
    if (!defFile.good()) {
        std::cerr << "ERROR: could not find file at path \"" << filePath << "\"" << std::endl;
        throw 1;
    }
    if (isEmpty(defFile)) {
        std::cerr << "ERROR: empty file given as definition" << std::endl;
        throw 1;
    }
    bool firstLine = !skipHead;

    while(std::getline(defFile, line)) {
        if (firstLine && line.find_first_not_of(' ') == std::string::npos) {
            if (defFile.eof()) {
                std::cerr << "ERROR: end of file reached before any definitions were found" << std::endl;
                throw 1;
            }
            continue;
        }
        std::istringstream lineStream(line);

        // parse definition head
        if(firstLine) {
            int stateCount;
            char lambda;
            char symbol;
            std::vector<char> alphabet;
            
            lineStream >> stateCount;
            lineStream >> lambda;

            while (lineStream >> symbol)
                alphabet.push_back(symbol);

            head.stateCount = stateCount;
            head.lambda = lambda;
            head.alphabet = alphabet;

            firstLine = false;
            continue;
        }

        // parse definition line
        DefinitionLine defLine;
        char acceptingChar, transitionCharacter;
        int to, from;
        std::vector<char> transitionCharacters;

        lineStream >> acceptingChar;
        lineStream >> from;
        lineStream >> to;
        while (lineStream >> transitionCharacter)
            transitionCharacters.push_back(transitionCharacter);

        defLine.accepting = acceptingChar == '+';
        defLine.from = from;
        defLine.to = to;
        defLine.transitionCharacters = transitionCharacters;

        lines.push_back(defLine);
    }

    def.head = head;
    def.lines = lines;

    defFile.close();

    return def;
}

std::ostream& operator<<(std::ostream& os, const DefinitionHeader& def) {
    os << "stateCount: " << def.stateCount << std::endl;
    os << "lambda: " << def.lambda << std::endl;
    os << "alphabet: " << def.alphabet << std::endl;
    
    return os;
}
std::ostream& operator<<(std::ostream& os, const DefinitionLine& def) {
    os << def.from << " => " << def.to << "\t" << def.transitionCharacters << "\t(" << (def.accepting ? '+' : '-') << ")" << std::endl;
    return os;
}
std::ostream& operator<<(std::ostream& os, const Definition& def) {
    os << "HEADER:" << std::endl;
    os << def.head;
    os << std::endl << "TRANSITIONS:" << std::endl;
    for(DefinitionLine line : def.lines) {
        os << line;
    }
    return os;
}





// NFA class
NFA::NFA(Definition def) {
    this->lambda = def.head.lambda;
    this->alphabet = def.head.alphabet;

    this->_constructFromDefinition(def);
}

void NFA::_constructFromDefinition(Definition def) {
    for(DefinitionLine line : def.lines) {
        if(!this->states.count(line.from)) {
            // add to states index
            StateInfo stateInfo;
            stateInfo.accepting = line.accepting;
            stateInfo.start = line.from == 0;
            this->states[line.from] = stateInfo;
        }
        // add to adjacency
        for(char transitionChar : line.transitionCharacters) {
            Transition t;
            t.to = line.to;
            t.token = transitionChar;
            this->adjacency[line.from].push_back(t);
        }
    }
}

std::vector<Transition> NFA::lambdaTransitions(int s) {
    return this->charTransitions(s, this->lambda);
}

std::vector<Transition> NFA::charTransitions(int s, char c) {
    std::vector<Transition> all = this->adjacency[s];
    std::vector<Transition> lambda;
    for (Transition t : all) {
        if (t.token == c)
            lambda.push_back(t);
    }
    return lambda;
}

state_set followLambda(NFA* nfa, state_set S) {
    std::vector<int> frontier;
    auto itr = S.begin();
    for (;itr != S.end(); itr++) {
        frontier.push_back(*itr);
    }
    while (frontier.size() > 0) {
        int t = frontier.back();
        frontier.pop_back();

        for(Transition tr : nfa->lambdaTransitions(t)) {
            // if frontier doesn't have state already in it
            if(S.find(tr.to) == S.end()) {
                S.insert(tr.to);
                frontier.push_back(tr.to);
            }
        }
    }
    return S;
}

state_set followChar(NFA* nfa, state_set S, char c) {
    state_set frontier;
    for(int state : S) {
        for(Transition t : nfa->charTransitions(state, c)) {
            frontier.insert(t.to);
        }
    }
    return frontier;
}

DFA NFA::toDFA() {
    transition_table<state_set> transitionTable;
    std::map<state_set, StateInfo> stateInfo;

    std::vector<state_set> L;

    // compute starting and accepting sets
    state_set startingStates;
    state_set acceptingStates;
    for(std::pair<int, StateInfo> s : this->states) {
        StateInfo i = s.second;
        if(i.start) startingStates.insert(s.first);
        if(i.accepting) acceptingStates.insert(s.first);
    }
    
    // merge starting states
    state_set dfaStarting = followLambda(this, startingStates);

    StateInfo dfaStartingInfo;
    dfaStartingInfo.start = true;
    state_set startingAccepting = stateIntersect(dfaStarting, acceptingStates);
    dfaStartingInfo.accepting = startingAccepting.size() > 0;
    stateInfo[dfaStarting] = dfaStartingInfo;

    L.push_back(dfaStarting);

    // iterate over table
    while (L.size() > 0) {
        state_set currentState = L.back();
        L.pop_back();

        for (char currentChar : this->alphabet) {
            state_set R = followLambda(this, followChar(this, currentState, currentChar));
            transitionTable[currentState][currentChar] = R;
            if(R.size() > 0 && stateInfo.count(R) == 0) {
                // assign info about R
                StateInfo Rinfo;
                Rinfo.start = false;
                state_set Raccepting = stateIntersect(R, acceptingStates);
                Rinfo.accepting = Raccepting.size() > 0;
                stateInfo[R] = Rinfo;

                // push onto stack to process it
                L.push_back(R);
            }
        }
    }


    // rewrite state names to simple integers
    std::map<state_set, int> nodeIdMap;
    int i = 0;
    for (std::pair<state_set, StateInfo> p : stateInfo) {
        nodeIdMap[p.first] = i;
        i++;
    }

    transition_table<int> sTransitionTable;
    for (std::pair<state_set, std::map<char, state_set>> tableRow : transitionTable) {
        int from = nodeIdMap[tableRow.first];
        for(std::pair<char, state_set> tableCell : tableRow.second) {
            char c = tableCell.first;
            int to = nodeIdMap[tableCell.second];
            if (tableCell.second.size() == 0) to = -1;
            sTransitionTable[from][c] = to;
        }
    }

    std::map<int, StateInfo> sStateInfo;
    for (std::pair<state_set, StateInfo> info : stateInfo) {
        sStateInfo[nodeIdMap[info.first]] = info.second;
    }

    DFA dfa(this->alphabet, sStateInfo, sTransitionTable);
    return dfa;
}



void NFABuilder::addEdge(int src, int dst, char c) {
    transitions[src][c] = dst;
}
void NFABuilder::addLambda(int src, int dst) {
    lambdas[src].insert(dst);
}
int NFABuilder::addState() {
    int lastState = nextState;
    nextState++;
    return lastState;
}
int NFABuilder::getAcceptingState() {
    return acceptingState;
}
void NFABuilder::setAcceptingState(int acceptingState) {
    this->acceptingState = acceptingState;
}

std::string NFABuilder::toGraphviz() {
    std::ostringstream oss;

    // print out the preamble
    oss << "digraph \"\"" << std::endl;
    oss << "{" << std::endl;
    oss << "fontname=\"Monospace\"" << std::endl;
    oss << "node [fontname=\"Monospace\"]" << std::endl;
    oss << "edge [fontname=\"Monospace\"]" << std::endl;
    oss << "rankdir=LR;" << std::endl;
    oss << std::endl;

    oss << "node [shape = doublecircle]; " << acceptingState << ";" << std::endl;
    oss << "node [shape = circle];" << std::endl;
    oss << std::endl;

    // add transition edges
    for (auto row : transitions) {
        for (auto edge : row.second) {
            char c = edge.first;
            oss << row.first << " -> " << edge.second;
            oss << " [label = \"";
            if ((c > 'A' && c < 'Z') || (c > 'a' && c < 'z')) oss << c;
            else oss << charToHex(c);
            oss << "\"];" << std::endl;
        }
    }

    // add lambda edges
    for (auto row : lambdas) {
        int from = row.first;
        for (int to : row.second) {
            oss << from << " -> " << to;
            oss << " [label = \"&#955;\"];" << std::endl;
        }
    }

    oss << "}" << std::endl;

    return oss.str();
}
