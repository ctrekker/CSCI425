#include <iostream>
#include <fstream>

#include <common/cfg.h>
#include <common/serialization.h>
#include <common/tree.h>

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
    // std::cout << cfg.followSet("E") << std::endl;
    // std::cout << cfg.derivesToLambda("C") << std::endl;
    // std::cout << cfg.derivesToLambda("K") << std::endl;
    // std::cout << cfg.printAllPredictSets() << std::endl;

    std::cout << cfg.stateTableLL1() << std::endl;
    std::pair<bool, ParseTree> matchResults = cfg.match("oparen plus two oparen mult three two two cparen cparen");
    // std::pair<bool, ParseTree> matchResults = cfg.match("oparen mult two three cparen");
    std::cout << "MATCH: " << (matchResults.first ? "TRUE" : "FALSE") << std::endl;
    cfg.printParseTree(matchResults.second);

    // ParseTree testTree;
    // int a = testTree.addNode(4);
    // int b = testTree.addNode(5);
    // int c = testTree.addNode(6);
    // int d = testTree.addNode(7);
    // int e = testTree.addNode(8);
    // testTree.addChild(a, b);
    // testTree.addChild(a, c);
    // testTree.addChild(c, d);
    // testTree.addChild(c, e);
    // std::cout << "TREE: " << testTree.toString() << std::endl;

    return 0;
}
