#pragma once

#include <vector>
#include <string>
#include "lexer.h"
#include "cfg.h"

std::vector<token> tokenizeRegex(std::string regex);
CFG llre();
