#include "lexer.h"
#include <sstream>
#include <iostream>
#include <iomanip>

Lexer::Lexer(std::vector<char> alphabet, std::vector<DFA> dfas, std::vector<std::string> tokens, std::vector<std::string> tokenData) {
    this->alphabet = alphabet;
    std::unordered_set<char> alphabetSet(alphabet.begin(), alphabet.end());
    this->alphabetSet = alphabetSet;
    this->dfas = dfas;
    this->tokens = tokens;
    this->tokenData = tokenData;
}

std::vector<token> Lexer::tokenize(const std::string& inputStr) {
    std::vector<token> tokenStream;
    std::vector<int> dfaStates(dfas.size(), 0);
    std::unordered_set<int> inactiveDfas;
    std::map<int, int> inactiveMatchedDfaEnds;

    // line counting variables
    int lineNum = 1;
    int linePos = 0;
    int startPos = 0;
    int pos = 0;
    while (pos < inputStr.length()) {
        while (inactiveDfas.size() < dfas.size() && pos < inputStr.length()) {
            char currentChar = inputStr.at(pos);

            if (alphabetSet.find(currentChar) == alphabetSet.end()) {
                std::cerr << "ERROR: character '" << currentChar << "' is not in the parse alphabet" << std::endl;
                throw 1;
            }
            for (int i=0; i<dfas.size(); i++) {
                if (inactiveDfas.find(i) != inactiveDfas.end()) continue;
                
                int currentState = dfaStates[i];
                int nextState = dfas[i].transition(currentState, currentChar);
                
                if (nextState == -1) {
                    inactiveDfas.insert(i);
                    if (dfas[i].isAccepting(currentState)) {
                        inactiveMatchedDfaEnds[i] = pos;
                    }
                }

                dfaStates[i] = nextState;
            }
            pos++;

            // if end is reached, check for accepting tokens
            if(pos == inputStr.length()) {
                for (int i=0; i<dfas.size(); i++) {
                    if (dfas[i].isAccepting(dfaStates[i])) {
                        inactiveMatchedDfaEnds[i] = pos;
                    }
                }
            }
        }

        int maxEnd = 0, maxToken = 0;
        for(std::pair<int, int> dfaEnds : inactiveMatchedDfaEnds) {
            // second disjunction condition checks for equal-length tokens, and gives precedence to 
            // those which occur earlier in the definition file
            if (dfaEnds.second > maxEnd || (dfaEnds.second == maxEnd && dfaEnds.first < maxToken)) {
                maxToken = dfaEnds.first;
                maxEnd = dfaEnds.second;
            }
        }

        inactiveMatchedDfaEnds.clear();
        inactiveDfas.clear();
        std::fill(dfaStates.begin(), dfaStates.end(), 0);

        token tok;
        tok.line = lineNum;
        tok.pos = linePos + 1;
        tok.type = tokens[maxToken];

        std::string altTokValue = tokenData[maxToken];
        if (altTokValue.length() == 0) {
            tok.value = inputStr.substr(startPos, maxEnd - startPos);
        }
        else {
            tok.value = altTokValue;
        }
        

        // update lineNum and linePos
        for (int i=startPos; i<maxEnd; i++) {
            char c = inputStr.at(i);
            linePos++;
            if (c == '\n') {
                lineNum++;
                linePos = 0;
            }
        }

        pos = maxEnd;
        startPos = pos;
        
        tokenStream.push_back(tok);
    }

    return tokenStream;
}

std::string _cleanSrcFormat(std::string val) {
    std::stringstream ss;
    for (int i=0; i<val.length(); i++) {
        char c = val.at(i);
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z' && c != 'x')) {
            ss << c;
        }
        else {
            ss << 'x' << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
    }
    return ss.str();
}

void printTokenStream(std::ostream& stream, const std::vector<token>& tokenStream) {
    for (int i=0; i<tokenStream.size(); i++) {
        token tok = tokenStream[i];
        stream << tok.type << " " << _cleanSrcFormat(tok.value) << " " << tok.line << " " << tok.pos << std::endl;
    }
}
