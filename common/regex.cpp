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
CHARRNG -> range char
         | lambda
)";
std::istringstream _llreSS(_llreCfg);
CFG _llre = CFG::parse(_llreSS);

CFG llre() {
    return _llre;
}

typedef std::map<std::string, int>& symbolmap;

void _sdt_nucleus(ParseTree &tree, int node, symbolmap smap) {
    std::vector<int>* children = tree.getChildren(node);
    int firstChild = children->at(0);

    if (tree.getLabel(firstChild) == smap["char"]) {
        int charrngNode = children->at(1);
        tree.removeChild(node, charrngNode);
        if (tree.getChildren(charrngNode)->size() != 0) {
            int char1Node = firstChild;
            int char2Node = tree.getChildren(charrngNode)->at(1);
            int dashNode = tree.getChildren(charrngNode)->at(0);
            tree.removeChild(node, char1Node);
            tree.removeChild(charrngNode, char2Node);
            tree.removeChild(charrngNode, dashNode);
            tree.addChild(dashNode, char1Node);
            tree.addChild(dashNode, char2Node);
            tree.addChild(node, dashNode);
        }
    }
    else if (tree.getLabel(firstChild) == smap["open"]) {
        int alt = children->at(1);
        tree.removeChild(node, children->at(2));
        tree.removeChild(node, children->at(1));
        tree.removeChild(node, children->at(0));
        for (int altc : *tree.getChildren(alt)) {
            tree.addChild(node, altc);
        }
    }
}

void _sdt_atom(ParseTree &tree, int node, symbolmap smap) {
    int nucleusParent = node;
    int nucleus = tree.getChildren(node)->at(0);
    int atommod = tree.getChildren(node)->at(1);
    tree.removeChild(node, atommod);
    tree.removeChild(node, nucleus);
    if (tree.getChildren(atommod)->size() != 0) {
        nucleusParent = tree.getChildren(atommod)->at(0);
        tree.removeChild(atommod, nucleusParent);
        tree.addChild(node, nucleusParent);
    }
    for (int nchild : *tree.getChildren(nucleus)) {
        tree.addChild(nucleusParent, nchild);
    }
}

void _sdt_seqlist(ParseTree &tree, int node, symbolmap smap) {
    std::vector<int> *children = tree.getChildren(node);
    if (children->size() == 0) return;
    
    int atom = children->at(0);
    int seqlist = children->at(1);
    int atomchild = tree.getChildren(atom)->at(0);

    tree.removeChild(atom, atomchild);
    tree.removeChild(node, atom);
    tree.removeChild(node, seqlist);

    tree.addChild(node, atomchild);
    std::vector<int> seqchildren = *tree.getChildren(seqlist);
    for (int child : seqchildren) {
        tree.addChild(node, child);
    }
}

void _sdt_altlist(ParseTree &tree, int node, symbolmap smap) {
    std::vector<int> *children = tree.getChildren(node);
    if (children->size() == 0) return;

    // if size is > 0, then a pipe is present
    int pipe = children->at(0);
    int seq = children->at(1);
    int altlist = children->at(2);

    tree.removeChild(node, altlist);
    tree.removeChild(node, seq);
    tree.addChild(pipe, seq);

    if (tree.getChildren(altlist)->size() > 0) {
        int altlistchild = tree.getChildren(altlist)->at(0);
        std::vector<int> altchildren = *tree.getChildren(altlistchild);
        for (int achild : altchildren) {
            tree.removeChild(altlistchild, achild);
            tree.addChild(pipe, achild);
        }
    }
}

void _sdt_alt(ParseTree &tree, int node, symbolmap smap) {
    std::vector<int> *children = tree.getChildren(node);
    if (children->size() == 0) return;

    int seq = children->at(0);
    int altlist = children->at(1);

    tree.removeChild(node, altlist);

    if (tree.getChildren(altlist)->size() != 0) {
        int altchild = tree.getChildren(altlist)->at(0);
        tree.removeChild(altlist, altchild);
        tree.addChild(node, altchild);
        tree.insertChild(altchild, seq, 0);
        tree.removeChild(node, seq);
    }
}

void _sdt_re(ParseTree &tree, int node, symbolmap smap) {
    std::vector<int> *children = tree.getChildren(node);
    int alt = children->at(0);
    tree.removeChild(node, children->at(1));
    tree.removeChild(node, alt);
    tree.addChild(node, tree.getChildren(alt)->at(0));
}

ParseTree parseRegex(std::string regex) {
    std::vector<token> regexTokens = tokenizeRegex(regex);

    std::map<std::string, sdtcallback> translationMap;
    translationMap["NUCLEUS"] = &_sdt_nucleus;
    translationMap["ATOM"]    = &_sdt_atom;
    translationMap["SEQLIST"] = &_sdt_seqlist;
    translationMap["SEQ"]     = &_sdt_seqlist;
    translationMap["ALTLIST"] = &_sdt_altlist;
    translationMap["ALT"]     = &_sdt_alt;
    translationMap["RE"]      = &_sdt_re;

    std::pair<bool, ParseTree> result = _llre.match(regexTokens, translationMap);
    return result.second;
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
                    tokType = "range";
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