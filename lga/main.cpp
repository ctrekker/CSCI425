#include <iostream>
#include <fstream>

#include <common/serialization.h>
#include <common/cfg.h>
#include <common/lexer.h>


int main(int argc, char** argv) {
    std::ifstream cfgStream(argv[1]);
    CFG cfg = CFG::parse(cfgStream);

    std::cout << cfg.formatForLGA() << std::endl;

    std::vector<token> tokens = readTokenFile(argv[2]);
    for (token t : tokens) {
        std::cout << "(" << t.type << "," << t.value << "), ";
    }
    std::cout << std::endl;

    std::pair<bool, ParseTree> matchResults = cfg.match(tokens);
    // std::pair<bool, ParseTree> matchResults = cfg.match("oparen mult two three cparen");
    std::cout << "MATCH: " << (matchResults.first ? "TRUE" : "FALSE") << std::endl;
    cfg.printParseTree(matchResults.second);
    cfg.saveGraphvizTree(argv[3], matchResults.second);

    return 0;
}