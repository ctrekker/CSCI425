#include <iostream>
#include <fstream>

#include "cfg.h"
#include <common/serialization.h>

void printHelp() {
    std::cout << "USAGE:" << std::endl;
    std::cout << "\tCFG [definition_path]" << std::endl;
}

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "ERROR: expected cfg file path" << std::endl;
        printHelp();
    }
    std::string cfgFile = argv[1];

    std::ifstream cfgStream(cfgFile);
    CFG cfg = CFG::parse(cfgStream);

    std::cout << cfg.formatForLGA() << std::endl;
    // for (std::string sym : cfg.getSymbols()) {
    //     if(!cfg.isTerminal(sym)) {
    //         std::cout << sym << ":" << std::endl;
    //         std::cout << "FIRST: " << cfg.toStrSyms(cfg.firstSet(sym)) << std::endl;
    //         std::cout << "FOLLOW: " << cfg.toStrSyms(cfg.followSet(sym)) << std::endl << std::endl;
    //     }
    // }
    // for (int nonterm : cfg.getNonterminals()) {

    // }
    std::cout << cfg.stateTableLL1() << std::endl;
    // cfg._testFunction();

    return 0;
}
