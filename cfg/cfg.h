#pragma once

#include <map>
#include <vector>
#include <istream>
#include <set>

typedef std::vector<int> GrammarRule;

class CFG {
    private:
    int terminalThreshold; // all i > terminalThreshold implies i is a terminal
    int totalSymbols;
    int goalSymbol = -1;
    int lambdaSymbol;
    int endSymbol;

    std::map<std::string, int> symbolMap;
    std::map<int, std::string> reverseSymbolMap;
    std::map<int, std::vector<GrammarRule>> rules;

    inline bool isTerminal(int sym);

    public:
    static CFG parse(std::istream &is);

    bool derivesToLambda(int nonterminal);
    bool derivesToLambda(std::string nonterminalName);
    bool derivesToLambda(int nonterminal, std::set<GrammarRule> &visited);

    std::set<int> firstSet(std::string sym);
    std::set<int> firstSet(std::vector<int> str);
    std::set<int> firstSet(std::vector<int> str, std::set<int> visited);

    std::set<int> followSet(std::string sym);
    std::set<int> followSet(int x);
    std::set<int> followSet(int x, std::set<int> visited);

    std::string formatForLGA();
};

