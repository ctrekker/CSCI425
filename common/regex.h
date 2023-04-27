#pragma once

#include <vector>
#include <string>
#include "lexer.h"
#include "cfg.h"
#include "nfa.h"

std::vector<token> tokenizeRegex(std::string regex);
ParseTree parseRegex(std::string regex);
NFABuilder nfaRegex(ParseTree& ast, std::vector<char> alphabet, std::map<int, std::string>& rsmap);
CFG llre();
