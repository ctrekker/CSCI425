#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <common/serialization.h>
#include <common/lexer.h>

struct toktable {
    std::string path;
    std::string token;
    std::string data;  // defaults to empty string if data should be matched token
};

struct tokdefs {
    std::vector<char> alphabet;
    std::vector<toktable> tables;
};

tokdefs readTokenDefinitions(std::string path) {
    std::ifstream defFile(path);

    std::string alphabetDef;
    getline(defFile, alphabetDef);

    std::vector<char> alphabet;
    for (int i=0; i<alphabetDef.size(); i++) {
        char c = alphabetDef.at(i);
        if (c == ' ') continue;
        if (c == 'x') {
            char d2 = alphabetDef.at(i+1);
            char d1 = alphabetDef.at(i+2);
            i+=2;

            int x;   
            std::stringstream ss;
            ss << std::hex << d2 << d1;
            ss >> x;

            alphabet.push_back((char)x);
        }
        else {
            alphabet.push_back(c);
        }
    }

    std::vector<toktable> tables;
    std::string line;
    while (std::getline(defFile, line))
    {
        std::istringstream iss(line);
        
        std::string tableFile;
        std::string tokenName;
        std::string tokenData;
        iss >> tableFile;
        iss >> tokenName;
        iss >> tokenData;
        if(tableFile.size() == 0 || tokenName.size() == 0) continue;

        toktable tab;
        tab.path = tableFile;
        tab.token = tokenName;
        tab.data = tokenData;
        tables.push_back(tab);
    }

    defFile.close();

    tokdefs def;
    def.alphabet = alphabet;
    def.tables = tables;
    return def;
}

void printHelp() {
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tLUTHER [DEFINITION_PATH] [PROGRAM_SRC] [TOKEN_OUTPUT_PATH]" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "ERROR: expected lexer token definition file path in argument 1" << std::endl;
        printHelp();
    }
    if (argc < 3) {
        std::cout << "ERROR: expected program source file path in argument 2" << std::endl;
        printHelp();
    }
    if (argc < 4) {
        std::cout << "ERROR: expected token output file path in argument 3" << std::endl;
        printHelp();
    }

    std::string defFile = argv[1];
    std::string srcFile = argv[2];
    std::string tokFile = argv[3];

    try {
        tokdefs def = readTokenDefinitions(defFile);
        std::vector<DFA> dfas;
        std::vector<std::string> tokens;
        std::vector<std::string> tokenData;
        for (toktable tab : def.tables) {
            DFA dfa = DFA::readTableFromAssignmentOutput(tab.path, def.alphabet);
            dfas.push_back(dfa);
            tokens.push_back(tab.token);
            tokenData.push_back(tab.data);
        }

        Lexer lex(def.alphabet, dfas, tokens, tokenData);
        std::ifstream srcStream(srcFile);
        std::stringstream srcBuf;
        srcBuf << srcStream.rdbuf();
        std::vector<token> tokenStream = lex.tokenize(srcBuf.str());

        std::ofstream tokenOutput(tokFile);
        printTokenStream(tokenOutput, tokenStream);
        tokenOutput.close();
    } catch(int e) {
        return e;
    }

    return 0;
}
