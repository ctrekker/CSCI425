#include <sstream>
#include "regex.h"
#include "cfg.h"
#include "nfa.h"

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
typedef std::map<int, std::string>& rsymbolmap;

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


class _RegexToNFA {
    private:
        ParseTree ast;
        std::vector<char> alphabet;
        std::map<int, std::string> rsmap;
        NFABuilder* nfa;

        void lambdaWrap(int node, int src, int dst);
        void processChild(int node, int src, int dst);
        void processSeq(int node, int src, int dst);
        void processPipe(int node, int src, int dst);
        void processKleene(int node, int src, int dst);
        void processPlus(int node, int src, int dst);
        void processChar(int node, int src, int dst);
        void processDot(int node, int src, int dst);
        void processRange(int node, int src, int dst);
    
    public:
        _RegexToNFA(ParseTree ast, std::vector<char> alphabet, std::map<int, std::string> rsmap);

        NFABuilder convert(int node);
};

_RegexToNFA::_RegexToNFA(ParseTree ast, std::vector<char> alphabet, std::map<int, std::string> rsmap) {
    this->ast = ast;
    this->alphabet = alphabet;
    this->rsmap = rsmap;
}

void _RegexToNFA::processChild(int node, int src, int dst) {
    int label = ast.getLabel(node);
    std::string strLabel = rsmap[label];
    std::cout << strLabel << std::endl;

    if (strLabel == "RE") {
        processChild(ast.getChildren(node)->at(0), src, dst);
    }
    else if (strLabel == "SEQ") {
        processSeq(node, src, dst);
    }
    else if (strLabel == "pipe") {
        processPipe(node, src, dst);
    }
    else if (strLabel == "kleene") {
        processKleene(node, src, dst);
    }
    else if (strLabel == "plus") {
        processPlus(node, src, dst);
    }
    else if (strLabel == "char") {
        processChar(node, src, dst);
    }
    else if (strLabel == "range") {
        processRange(node, src, dst);
    }
    else if (strLabel == "dot") {
        processDot(node, src, dst);
    }
}
void _RegexToNFA::lambdaWrap(int node, int src, int dst) {
    int left = nfa->addState();
    int right = nfa->addState();
    nfa->addLambda(src, left);
    nfa->addLambda(right, dst);
    processChild(node, left, right);
}
void _RegexToNFA::processSeq(int node, int src, int dst) {
    int childdest;
    for (int child : *ast.getChildren(node)) {
        childdest = nfa->addState();
        lambdaWrap(child, src, childdest);
        src = childdest;
    }
    nfa->addLambda(childdest, dst);
}
void _RegexToNFA::processPipe(int node, int src, int dst) {
    for (int child : *ast.getChildren(node)) {
        lambdaWrap(child, src, dst);
    }
}
void _RegexToNFA::processKleene(int node, int src, int dst) {
    processChild(ast.getChildren(node)->at(0), src, dst);
    nfa->addLambda(dst, src);
}
void _RegexToNFA::processPlus(int node, int src, int dst) {
    int intermediate = nfa->addState();
    int child = ast.getChildren(node)->at(0);
    processChild(child, src, intermediate);
    processKleene(node, intermediate, dst);
}
void _RegexToNFA::processChar(int node, int src, int dst) {
    // in theory metadata tags should NEVER be longer than 1 character for regular expressions
    char c = ast.getMetadata(node)->value.at(0);
    nfa->addEdge(src, dst, c);
}
void _RegexToNFA::processRange(int node, int src, int dst) {
    int leftChild = ast.getChildren(node)->at(0);
    int rightChild = ast.getChildren(node)->at(1);
    char asciiStart = ast.getMetadata(leftChild)->value.at(0);
    char asciiEnd = ast.getMetadata(rightChild)->value.at(0);
    for (char c = asciiStart; c <= asciiEnd; c++) {
        nfa->addEdge(src, dst, c);
    }
}
void _RegexToNFA::processDot(int node, int src, int dst) {
    for (char c : alphabet) {
        nfa->addEdge(src, dst, c);
    }
}

NFABuilder _RegexToNFA::convert(int node) {
    NFABuilder nfa;
    int start = nfa.addState();
    int end = nfa.addState();
    nfa.setAcceptingState(end);

    this->nfa = &nfa;

    processChild(node, start, end);
    return nfa;
}


// converts a regex AST into an NFA implementation
NFABuilder nfaRegex(ParseTree& ast, std::vector<char> alphabet, rsymbolmap rsmap) {
    _RegexToNFA r(ast, alphabet, rsmap);
    return r.convert(ast.rootNode());
}
