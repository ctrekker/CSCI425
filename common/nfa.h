#pragma once
#include <map>
#include <vector>

#include "serialization.h"
#include "dfa.h"

struct DefinitionHeader {
    int stateCount;
    char lambda;
    std::vector<char> alphabet;
};

struct DefinitionLine {
    bool accepting;
    int from;
    int to;
    std::vector<char> transitionCharacters;
};

struct Definition {
    DefinitionHeader head;
    std::vector<DefinitionLine> lines;
};

Definition readDefinition(std::string filePath, bool skipHead=false);


std::ostream& operator<<(std::ostream& os, const DefinitionHeader& def);
std::ostream& operator<<(std::ostream& os, const DefinitionLine& def);
std::ostream& operator<<(std::ostream& os, const Definition& def);



class NFA {
    private:
        std::map<int, std::vector<Transition>> adjacency;
        std::map<int, StateInfo> states;
        char lambda;
        std::vector<char> alphabet;

        void _constructFromDefinition(Definition def);
    public:
        NFA(Definition def);
        DFA toDFA();
        std::vector<Transition> lambdaTransitions(int s);
        std::vector<Transition> charTransitions(int s, char c);
};
