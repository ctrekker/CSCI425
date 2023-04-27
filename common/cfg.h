#pragma once

#include <map>
#include <vector>
#include <istream>
#include <set>
#include "tree.h"
#include "lexer.h"

typedef std::vector<int> GrammarRule;
typedef void (*sdtcallback)(ParseTree&, int, std::map<std::string, int>&);

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

    public:
    static CFG parse(std::istream &is);
    bool isTerminal(std::string symStr);
    inline bool isTerminal(int sym);
    std::vector<std::string> getSymbols();
    std::map<std::string, int> getSymbolMap();
    std::map<int, std::string> getReverseSymbolMap();

    std::vector<std::string> toStrSyms(std::set<int> arr);
    std::vector<std::string> toStrSyms(std::vector<int> arr);

    bool derivesToLambda(int nonterminal);
    bool derivesToLambda(std::string nonterminalName);
    bool derivesToLambda(int nonterminal, std::set<GrammarRule> &visited);
    bool derivesToLambda(GrammarRule rule);

    std::set<int> firstSet(std::string sym);
    std::set<int> firstSet(std::vector<int> str);
    std::set<int> firstSet(std::vector<int> str, std::set<int> visited);

    std::set<int> followSet(std::string sym);
    std::set<int> followSet(int x);
    std::set<int> followSet(int x, std::set<int> visited);

    std::set<int> predictSet(int sym, GrammarRule rule);

    std::pair<bool, ParseTree> match(std::string str);
    std::pair<bool, ParseTree> match(std::vector<token> tokenStream);
    std::pair<bool, ParseTree> match(std::vector<token> tokenStream, std::map<std::string, sdtcallback> translations);
    std::string printAllPredictSets();

    std::map<int, std::map<int, int>> stateTableLL1();
    
    // syntax directed translation
    void performTranslation(std::map<int, sdtcallback> translations, ParseTree &tree, int node);

    std::string formatForLGA();
    void printParseTree(ParseTree t);
    void saveGraphvizTree(std::string file, ParseTree t);
};
