#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <common/serialization.h>
#include <common/lexer.h>
#include <common/cfg.h>
#include <common/regex.h>
#include <common/nfa.h>

struct toktable {
    std::string regex;
    std::string token;
    std::string data;  // defaults to empty string if data should be matched token
};

struct tokdefs {
    std::vector<char> alphabet;
    std::vector<toktable> tables;
};

tokdefs readTokenConfig(std::string path) {
    std::ifstream defFile(path);
    if (!defFile.good()) {
        std::cerr << "ERROR: cannot access token configuration at \"" << path << "\"" << std::endl;
        throw 1;
    }

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
        
        std::string tokenRegex;
        std::string tokenName;
        std::string tokenData;
        iss >> tokenRegex;
        iss >> tokenName;
        iss >> tokenData;
        if(tokenRegex.size() == 0 || tokenName.size() == 0) continue;

        toktable tab;
        tab.regex = tokenRegex;
        tab.token = tokenName;
        tab.data = readHexASCII(tokenData);
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
    std::cout << "\tWRECK [CONFIG_PATH] [DEFINITION_PATH]" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "ERROR: expected lexer token definition file path in argument 1" << std::endl;
        printHelp();
        return 1;
    }
    if (argc < 3) {
        std::cerr << "ERROR: expected program source file path in argument 2" << std::endl;
        printHelp();
        return 1;
    }

    std::string configFile = argv[1];
    std::string scanFile = argv[2];

    std::map<int, std::string> rsmap = llre().getReverseSymbolMap();

    tokdefs def;
    try {
        def = readTokenConfig(configFile);
    } catch(int e) {
        return e;
    }

    for (toktable tab : def.tables) {
        std::cout << "TOK: " << tab.token << "; REGEX: " << tab.regex << std::endl;
        if (tab.data.size() > 0) std::cout << "\tDATA: " << tab.data << std::endl;

        ParseTree regexAst = parseRegex(tab.regex);
        NFABuilder nfa = nfaRegex(regexAst, def.alphabet, rsmap);
        Definition nfaDef = nfa.toDefinition(def.alphabet);
        
        std::ostringstream oss;
        oss << tab.token << ".nfa";
        std::ofstream defOutput(oss.str());
        writeDefinition(defOutput, nfaDef);
        defOutput.close();
    }

    // write output scan table file
    std::ofstream scan(scanFile);
    for (char c : def.alphabet) {
        scan << charToHexIfNecessary(c);
    }
    scan << std::endl;
    for (toktable table : def.tables) {
        scan << table.token << ".tt " << table.token << " " << table.data << std::endl;
    }
    scan.close();
    

    // EXAMPLE USAGE
    // ParseTree result = parseRegex("Q-T(a.)*g+");
    // llre().saveGraphvizTree("testplot.gv.txt", result);

    

    return 0;
}
