#include "cfg.h"

#include "serialization.h"
#include "setUtils.h"
#include "tree.h"
#include "lexer.h"

#include <istream>
#include <fstream>
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
    lambdaSymbol = symbolMap["lambda"];
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

bool CFG::isTerminal(std::string symStr) {
    return isTerminal(symbolMap[symStr]);
}
inline bool CFG::isTerminal(int sym) {
    return sym > terminalThreshold && sym != lambdaSymbol;
}

std::vector<std::string> CFG::toStrSyms(std::set<int> arr) {
    std::vector<std::string> out;
    for (int i : arr) {
        out.push_back(reverseSymbolMap[i]);
    }
    return out;
}
std::vector<std::string> CFG::toStrSyms(std::vector<int> arr) {
    std::vector<std::string> out;
    for (int i : arr) {
        out.push_back(reverseSymbolMap[i]);
    }
    return out;
}

std::vector<std::string> CFG::getSymbols() {
    std::vector<std::string> out;
    for (std::pair<std::string, int> p : symbolMap) {
        out.push_back(p.first);
    }
    return out;
}

std::map<std::string, int> CFG::getSymbolMap() {
    return symbolMap;
}
std::map<int, std::string> CFG::getReverseSymbolMap() {
    return reverseSymbolMap;
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
bool CFG::derivesToLambda(GrammarRule rule) {
    for (int sym : rule) {
        if (sym == lambdaSymbol) continue;
        if (isTerminal(sym) || !derivesToLambda(sym)) return false;
    }
    return true;
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
        std::set<int> subFirst = firstSet(str, visited);
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

    // std::cout << "X: " << x << std::endl;

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
                    std::cout << pi << std::endl;
                    std::set<int> first = firstSet(pi);
                    unionMutating(follow, first);
                }
                bool allSubsequentLambdas = true;
                // for (auto itr2 = itr; itr2 < rule.end(); itr2++) {
                //     if (!isTerminal(*itr2)) {
                //         allSubsequentLambdas = false;
                //         break;
                //     }
                // }
                for (auto itr2 = itr; itr2 < rule.end(); itr2++) {
                    if (!derivesToLambda(*itr2)) {
                        allSubsequentLambdas = false;
                        break;
                    }
                }
                if(allSubsequentLambdas && visited.find(rulePair.first) == visited.end()) {
                    std::cout << x << ",SUBF: " << rulePair.first << std::endl;
                    std::set<int> subFollow = followSet(rulePair.first, visited);
                    unionMutating(follow, subFollow);
                }
            }
        }
    }

    return follow;
}

std::set<int> CFG::predictSet(int sym, GrammarRule rule) {
    std::set<int> predict = firstSet(rule);
    if (derivesToLambda(rule)) {
        std::set<int> follow = followSet(sym);
        unionMutating(predict, follow);
    }
    return predict;
}

std::map<int, std::map<int, int>> CFG::stateTableLL1() {
    // maps NONTERM -> (TERM -> LOCALRULE)
    std::map<int, std::map<int, int>> table;
    for (int i = 0; i <= terminalThreshold; i++) {
        std::map<int, int> tableRow;
        for (int ruleNum=0; ruleNum<rules[i].size(); ruleNum++) {
            std::set<int> predict = predictSet(i, rules[i][ruleNum]);
            for (int psym : predict) {
                if (tableRow.find(psym) != tableRow.end()) {
                    std::cerr << "ERROR: CFG not compatible with LL(1) parse table" << std::endl;
                    std::cout << i << ": " << tableRow << " , " << predict << std::endl;
                    throw 1;
                }
                tableRow[psym] = ruleNum;
            }
        }
        table[i] = tableRow;
    }
    return table;
}

std::pair<bool, ParseTree> CFG::match(std::string str) {
    std::vector<token> tokenStream;
    int startPos = 0;
    for (int i=0; i<=str.length(); i++) {
        if (str[i] == ' ' || i == str.length()) {
            std::string strTok = str.substr(startPos, i - startPos);
            if (symbolMap.find(strTok) == symbolMap.end()) {
                std::cerr << "ERROR: unexpected token '" << strTok << "'; symbol is not in grammar alphabet" << std::endl;
                throw 1;
            }
            token t;
            t.type = symbolMap[strTok];
            t.value = "";
            tokenStream.push_back(t);
            startPos = i + 1;
        }
    }
    return match(tokenStream);
}

std::pair<bool, ParseTree> CFG::match(std::vector<token> tokenStream) {
    std::map<std::string, sdtcallback> emptyTranslations;
    return match(tokenStream, emptyTranslations);
}

std::pair<bool, ParseTree> CFG::match(std::vector<token> tokenStream, std::map<std::string, sdtcallback> translations) {
    token t;
    t.type = "$";
    t.value = "";
    tokenStream.push_back(t);

    GrammarRule g;
    g.push_back(8);
    std::cout << followSet(5) << std::endl;
    std::cout << reverseSymbolMap << std::endl;
    // throw 0;

    std::map<int, std::map<int, int>> ll1 = stateTableLL1();

    std::map<int, sdtcallback> encodedTranslations;
    for (std::pair<std::string, sdtcallback> translatePair : translations) {
        encodedTranslations[symbolMap[translatePair.first]] = translatePair.second;
    }


    
    // std::cout << derivesToLambda(3) << std::endl;
    std::cout << ll1 << std::endl;

    ParseTree parseTree;
    int parseRoot = parseTree.addNode(-1, EMPTY_METADATA);
    int parseNode = parseRoot;

    std::vector<int> derivationStack;
    derivationStack.push_back(goalSymbol);
    int stackPos = 0;

    while (stackPos < tokenStream.size()) {
        int s = derivationStack[derivationStack.size() - 1];
        derivationStack.pop_back();

        std::cout << s << ":" << derivationStack << " <==> " << stackPos << std::endl;
        // std::cout << "SYM: " << c << "S"

        if (s == -1) {
            parseNode = parseTree.getParent(parseNode);
            continue;
        }

        int nextParseNode = parseTree.addNode(s, EMPTY_METADATA);
        parseTree.addChild(parseNode, nextParseNode);
        parseNode = nextParseNode;

        int c = symbolMap[tokenStream[stackPos].type];
        std::map<int, int> tableRow = ll1[s];
        if (tableRow.find(c) == tableRow.end()) {
            std::cerr << "ERROR: unexpected token \'" << reverseSymbolMap[c] << "\' in position " << stackPos << std::endl;
            std::cout << derivationStack << std::endl;
            return std::make_pair(false, parseTree);
        }
        int applyRule = tableRow[c];
        GrammarRule rule = rules[s][applyRule];
        derivationStack.push_back(-1); // rule term: signifies moving up to parent node
        for (int i=rule.size()-1; i>=0; i--) derivationStack.push_back(rule[i]);
        for (int i=derivationStack.size()-1; i>=0; i--) {
            if (derivationStack[i] == -1) {
                performTranslation(encodedTranslations, parseTree, parseNode);
                parseNode = parseTree.getParent(parseNode);
                derivationStack.pop_back();
            }
            else if (derivationStack[i] == symbolMap[tokenStream[stackPos].type]) {
                tree_metadata meta;
                meta.value = tokenStream[stackPos].value;
                int tokenNode = parseTree.addNode(symbolMap[tokenStream[stackPos].type], meta);
                parseTree.addChild(parseNode, tokenNode);

                stackPos++;
                derivationStack.pop_back();
            }
            else if (derivationStack[i] == lambdaSymbol) derivationStack.pop_back();
            else break;
        }
    }

    // set new root to user-defined goal nonterminal
    parseTree.setRoot(parseTree.getChildren(parseTree.rootNode())->at(0));

    return std::make_pair(derivationStack.size() == 0, parseTree);
}

void CFG::performTranslation(std::map<int, sdtcallback> translations, ParseTree &tree, int node) {
    int label = tree.getLabel(node);
    if (translations.find(label) != translations.end()) {
        (*translations[label])(tree, node, symbolMap);
    }
}

std::string CFG::printAllPredictSets() {
    std::stringstream ss;
    for (int i=0; i<=terminalThreshold; i++) {
        for (GrammarRule rule : rules[i]) {
            ss << "PREDICT " << reverseSymbolMap[i] << " -> ";
            for (int sym : rule) {
                ss << reverseSymbolMap[sym] << " ";
            }
            ss << std::endl;
            ss << "\t" << toStrSyms(predictSet(i, rule)) << std::endl;
        }
        ss << std::endl;
    }

    return ss.str();
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

void CFG::printParseTree(ParseTree t) {
    std::cout << t.toString(reverseSymbolMap) << std::endl;
}

void CFG::saveGraphvizTree(std::string file, ParseTree t) {
    std::ofstream fileOut(file);
    std::map<int, std::string> rsm = reverseSymbolMap;
    rsm[-1] = "ROOT";
    fileOut << t.toGraphviz(rsm);
    fileOut.close();
}
