#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <common/serialization.h>
#include <common/nfa.h>

void printHelp() {
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tNFAMATCH [DEFINITION_PATH] [DFA_OUTPUT_PATH] [MATCH_STRINGS...]" << std::endl;
}

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "ERROR: expected NFA definition file path in argument 1" << std::endl;
        printHelp();
        return 1;
    }
    if(argc < 3) {
        std::cout << "ERROR: expected DFA definition output file path in argument 2" << std::endl;
        printHelp();
        return 1;
    }
    std::vector<std::string> matchCases;
    for(int i=3; i<argc; i++) {
        matchCases.push_back(argv[i]);
    }
    std::string nfaFile = argv[1];
    std::string dfaFile = argv[2];

    Definition nfaDef;
    try {
        nfaDef = readDefinition(nfaFile);
    } catch(int code) {
        return code;
    }

    NFA nfa(nfaDef);
    DFA dfa = nfa.toDFA();
    dfa.optimize();

    // perform matching
    for (std::string matchCase : matchCases) {
        std::cout << "OUTPUT ";

        auto match = dfa.match(matchCase);
        if (match.first)
            std::cout << ":M:";
        else 
            std::cout << match.second;
        std::cout << std::endl;
    }

    // output the optimized DFA transition table
    std::ofstream outputFile(dfaFile);
    outputFile << dfa.formatTableForAssignmentOutput();
    outputFile.close();

    return 0;
}
