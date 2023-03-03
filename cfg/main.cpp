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

    return 0;
}