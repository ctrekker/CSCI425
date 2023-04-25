#include <sstream>
#include "regex.h"
#include "cfg.h"

const char* _llreCfg = R"(
     RE -> ALT $
    ALT -> SEQ ALTLIST
ALTLIST -> pipe SEQ ALTLIST
         | lambda
    SEQ -> ATOM SEQLIST
	     | lambda         
SEQLIST -> ATOM SEQLIST
		 | lambda
   ATOM -> NUCLEUS ATOMMOD
ATOMMOD -> kleene | plus | lambda 
NUCLEUS -> open ALT close
         | char CHARRNG
         | dot
CHARRNG -> dash char
         | lambda
)";
std::istringstream _llreSS(_llreCfg);
const CFG _llre = CFG::parse(_llreSS);

CFG llre() {
    return _llre;
}

std::vector<token> tokenizeRegex(std::string regex) {
    std::vector<token> tokStream;
    char c;
    bool controlChar = false;
    for (int i=0; i<regex.size(); i++) {
        c = regex.at(i);
        std::string v;
        v.push_back(c);
        token t;

        if (controlChar) {
            if (c == '\\') {
                t = create_token("char", "\\", 1, i);
            }
            else if (c == 's') {
                t = create_token("char", " ", 1, i);
            }
            controlChar = false;
        }
        else {
            if (c == '\\') {
                controlChar = true;
                continue;
            }

            std::string tokType;
            switch (c) {
                case '*':
                    tokType = "kleene";
                    break;
                case '+':
                    tokType = "plus";
                    break;
                case '|':
                    tokType = "pipe";
                    break;
                case '.':
                    tokType = "dot";
                    break;
                case '-':
                    tokType = "dash";
                    break;
                case '(':
                    tokType = "open";
                    break;
                case ')':
                    tokType = "close";
                    break;
                default:
                    tokType = "char";
                    break;
            }
            t = create_token(tokType, v, 1, i);
        }

        
        tokStream.push_back(t);
    }
    return tokStream;
}