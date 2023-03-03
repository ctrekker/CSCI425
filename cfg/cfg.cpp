#include "cfg.h"

#include <common/serialization.h>
#include <common/setUtils.h>

#include <istream>
#include <iostream>
#include <string>
#include <sstream>
#include <set>
#include <algorithm>

struct Symbol {
    std::string symbol;
    bool isTerminal;
};

CFG CFG::parse(std::istream &ss) {
    std::string token;

    std::set<std::string> terminals;
    std::set<std::string> nonterminals;

    std::vector<std::string> tokens;

    std::string previousToken;

    while (ss >> token) {
        tokens.push_back(token);

        if (token == "->") {
            nonterminals.insert(previousToken);
            if (terminals.find(previousToken) != terminals.end()) {
                terminals.erase(previousToken);
            }
        }
        else if (token == "|") {
            // don't do anything in this phase of the parse
        }
        else {
            if (nonterminals.find(token) == nonterminals.end()) {
                terminals.insert(token);
            }
        }

        previousToken = token;
    }

    int terminalThreshold = nonterminals.size() - 1;
    std::map<std::string, int> symbolMap;
    std::map<int, std::string> reverseSymbolMap;

    // construct the symbol maps
    int sid = 0;
    for (std::string nt : nonterminals) {
        symbolMap[nt] = sid;
        reverseSymbolMap[sid] = nt;
        sid++;
    }
    for (std::string t : terminals) {
        symbolMap[t] = sid;
        reverseSymbolMap[sid] = t;
        sid++;
    }

    std::map<int, std::vector<GrammarRule>> rules;
    int currentNonterminal = -1;
    int goalSymbol = -1;
    GrammarRule currentRule;
    std::cout << tokens << std::endl;
    for (std::string token : tokens) {
        if (token == "->" || token == "|") {
            if (token == "->") currentRule.pop_back();
            if (currentNonterminal != -1) {
                rules[currentNonterminal].push_back(currentRule);
                currentRule.clear();
            }

            if (token == "->") currentNonterminal = symbolMap[previousToken];
        }
        else {
            std::cout << token << ", " << currentRule << std::endl;
            currentRule.push_back(symbolMap[token]);
            if (token == "$") goalSymbol = currentNonterminal;
        }
        previousToken = token;
    }
    rules[currentNonterminal].push_back(currentRule);

    int lambdaSymbol;
    if (symbolMap.find("lambda") == symbolMap.end()) {
        symbolMap["lambda"] = sid;
        reverseSymbolMap[sid] = "lambda";
        sid++;
    }
    else {
        lambdaSymbol = symbolMap["lambda"];
    }
    if (symbolMap.find("$") == symbolMap.end()) {
        throw "ERROR: CFG must include $ symbol in goal nonterminal";
    }
    int endSymbol = symbolMap["$"];

    CFG cfg;
    cfg.terminalThreshold = terminalThreshold;
    cfg.totalSymbols = nonterminals.size() + terminals.size();
    cfg.goalSymbol = goalSymbol;
    cfg.lambdaSymbol = lambdaSymbol;  // id corresponding to `lambda`
    cfg.endSymbol = endSymbol;        // id corresponding to $
    cfg.symbolMap = symbolMap;
    cfg.reverseSymbolMap = reverseSymbolMap;
    cfg.rules = rules;

    return cfg;
}

inline bool CFG::isTerminal(int sym) {
    return sym > terminalThreshold && sym != lambdaSymbol;
}


bool CFG::derivesToLambda(std::string nonterminalName) {
    return derivesToLambda(symbolMap[nonterminalName]);
}
bool CFG::derivesToLambda(int nonterminal) {
    std::set<GrammarRule> visitedStack;  // start with empty stack
    return derivesToLambda(nonterminal, visitedStack);
}
bool CFG::derivesToLambda(int nonterminal, std::set<GrammarRule> &visited) {
    std::vector<GrammarRule> grammarRules = rules[nonterminal];
    for (GrammarRule rule : grammarRules) {
        // check if this rule has already been visited
        if (visited.find(rule) != visited.end()) {
            continue;
        }
        // check if this rule is simply "lambda"
        if (rule.size() == 1 && rule[0] == lambdaSymbol) {
            return true;
        }
        // check if this rule contains a terminal
        bool shouldContinue = false;
        for (int sym : rule) {
            if (isTerminal(sym)) {
                shouldContinue = true;
                break;
            }
        }
        if (shouldContinue) continue;

        // check if all nonterminals in grammar rule derive to lambda
        bool allDerivesToLambda = true;
        visited.insert(rule);
        for (int sym : rule) {
            if(!derivesToLambda(sym, visited)) {
                allDerivesToLambda = false;
                break;
            }
        }
        visited.erase(rule);
        if(allDerivesToLambda) {
            return true;
        }
    }
    return false;
}

std::set<int> CFG::firstSet(std::string sym) {
    std::vector<int> str;
    str.push_back(symbolMap[sym]);
    return firstSet(str);
}
std::set<int> CFG::firstSet(std::vector<int> str) {
    std::set<int> emptySet;
    return firstSet(str, emptySet);
}
std::set<int> CFG::firstSet(std::vector<int> str, std::set<int> visited) {
    const int x = str[0];
    std::set<int> first;
    if (isTerminal(x)) {
        first.insert(x);
        return first;
    }
    if (visited.find(x) == visited.end()) {
        visited.insert(x);

        for (GrammarRule rule : rules[x]) {
            std::set<int> subFirst = firstSet(rule, visited);
            unionMutating(first, subFirst);
        }
    }
    if (derivesToLambda(x) && str.size() > 1) {
        str.erase(str.begin());
        std::set<int> subFirst = firstSet(str);
        unionMutating(first, subFirst);
    }
    return first;
}

std::set<int> CFG::followSet(std::string sym) {
    return followSet(symbolMap[sym]);
}
std::set<int> CFG::followSet(const int x) {
    std::set<int> emptySet;
    return followSet(x, emptySet);
}
std::set<int> CFG::followSet(const int x, std::set<int> visited) {
    const std::set<int> emptySet;

    if (isTerminal(x)) {
        return emptySet;
    }

    visited.insert(x);
    std::set<int> follow;
    for (std::pair<int, std::vector<GrammarRule>> rulePair : rules) {
        for (GrammarRule rule : rulePair.second) {
            auto itr = std::find(rule.begin(), rule.end(), x);
            if (itr != rule.end()) {
                itr++;
                if (itr != rule.end()) {
                    std::vector<int> pi(rule.end() - itr);
                    copy(itr, rule.end(), pi.begin());
                    std::set<int> first = firstSet(pi, visited);
                    unionMutating(follow, first);
                }
                bool allSubsequentLambdas = true;
                for (auto itr2 = itr; itr2 < rule.end(); itr2++) {
                    if (!isTerminal(*itr2)) {
                        allSubsequentLambdas = false;
                        break;
                    }
                }
                if (allSubsequentLambdas) {
                    for (auto itr2 = itr; itr2 < rule.end(); itr2++) {
                        if (!derivesToLambda(*itr2)) {
                            allSubsequentLambdas = false;
                            break;
                        }
                    }
                }
                if(allSubsequentLambdas && visited.find(rulePair.first) == visited.end()) {
                    std::set<int> subFollow = followSet(rulePair.first, visited);
                    unionMutating(follow, subFollow);
                }
            }
        }
    }

    return follow;
}

std::string CFG::formatForLGA() {
    std::stringstream ss;
    ss << "Grammar Non-Terminals" << std::endl;
    for (int sym = 0; sym <= terminalThreshold; sym++) {
        ss << reverseSymbolMap[sym];
        if (sym < terminalThreshold) {
            ss << ", ";
        }
    }

    ss << std::endl;
    ss << "Grammar Symbols" << std::endl;
    std::vector<int> normalTerminals;
    for (int sym = 0; sym < totalSymbols; sym++) {
        if (reverseSymbolMap[sym] != "lambda") normalTerminals.push_back(sym);
    }

    int i = 0;
    for (int sym : normalTerminals) {
        ss << reverseSymbolMap[sym];
        if (i < normalTerminals.size() - 1) {
            ss << ", ";
        }
        i++;
    }
    ss << std::endl << std::endl;

    ss << "Grammar Rules" << std::endl;
    int ruleIdx = 1;
    for (std::pair<int, std::vector<GrammarRule>> rulesPair : rules) {
        for (GrammarRule rule : rulesPair.second) {
            ss << "(" << ruleIdx << ")\t" << reverseSymbolMap[rulesPair.first] << " -> ";
            for (int i=0; i<rule.size(); i++) {
                ss << reverseSymbolMap[rule[i]];
                if (i < rule.size() - 1) ss << " ";
            }
            ss << std::endl;

            ruleIdx++;
        }
    }
    ss << std::endl;

    ss << "Grammar Start Symbol or Goal: " << reverseSymbolMap[goalSymbol] << std::endl;

    return ss.str();
}
