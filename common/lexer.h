#pragma once

#include <vector>
#include <unordered_set>
#include "dfa.h"

struct token {
    std::string type;
    std::string value;
    int line;
    int pos;
};

class Lexer {
    private:
        std::vector<char> alphabet;
        std::unordered_set<char> alphabetSet;
        std::vector<DFA> dfas;
        std::vector<std::string> tokens;
        std::vector<std::string> tokenData;
    public:
        Lexer(std::vector<char> alphabet, std::vector<DFA> dfas, std::vector<std::string> tokens, std::vector<std::string> tokenData);
        std::vector<token> tokenize(const std::string& inputStr);
};

void printTokenStream(std::ostream& stream, const std::vector<token>& tokenStream);
std::vector<token> readTokenFile(std::string path);
std::string readHexASCII(std::string str);
