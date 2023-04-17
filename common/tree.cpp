#include "tree.h"

#include <sstream>
#include <iostream>
#include "serialization.h"

inline int ParseTree::rootNode() {
    return 0;
}

std::vector<int>* ParseTree::getChildren(int nodeId) {
    return &adj[nodeId];
}

int ParseTree::getParent(int nodeId) {
    return parentMap[nodeId];
}

inline int ParseTree::getLabel(int node) {
    return labels[node];
}

int ParseTree::addNode(int label, tree_metadata meta) {
    std::vector<int> empty;
    adj.push_back(empty);
    labels.push_back(label);
    metadata.push_back(meta);
    return adj.size() - 1;
}

void ParseTree::addChild(int parent, int child) {
    adj[parent].push_back(child);
    parentMap[child] = parent;
}

tree_metadata* ParseTree::getMetadata(int node) {
    return &metadata[node];
}

bool ParseTree::isLeaf(int node) {
    return adj[node].size() == 0;
}


void _printCount(std::stringstream &ss, std::string str, int count) {
    for(int i=0; i<count; i++) {
        ss << str;
    }
}
std::string ParseTree::toString() {
    std::map<int, std::string> emptyMap;
    return toString(emptyMap, rootNode(), 0);
}
std::string ParseTree::toString(std::map<int, std::string> labelMap) {
    return toString(labelMap, rootNode(), 0);
}
std::string ParseTree::toString(std::map<int, std::string> labelMap, int node, int level) {
    std::stringstream ss;

    for (int i=0; i<adj[node].size(); i++) {
        int n = adj[node][i];
        int nl = labels[n];
        _printCount(ss, "  ", level);
        if (labelMap.find(nl) != labelMap.end()) {
            ss << labelMap[nl] << std::endl;
        }
        else {
            ss << nl << std::endl;
        }

        if (!isLeaf(n)) {
            _printCount(ss, "  ", level);
            ss << "(" << std::endl;
            ss << toString(labelMap, n, level+1);
            _printCount(ss, "  ", level);
            ss << ")" << std::endl;
        }
    }

    return ss.str();
}

void _gvRecurse(std::ostream& os, ParseTree *tree, std::map<int, std::string> &labelMap, int node, int *idx) {
    os << "n" << *idx << " [label=\"" << labelMap[tree->getLabel(node)] << "\"] ;" << std::endl;
    int parentIdx = *idx;
    (*idx)++;
    int childIdx;

    for (int child : *tree->getChildren(node)) {
        childIdx = *idx;
        _gvRecurse(os, tree, labelMap, child, idx);
        os << "n" << parentIdx << " -- " << "n" << childIdx << " ;" << std::endl;
    }
}

std::string ParseTree::toGraphviz(std::map<int, std::string> labelMap) {
    std::ostringstream oss;

    // print out the preamble
    oss << "graph \"\"" << std::endl;
    oss << "{" << std::endl;
    oss << "fontname=\"Monospace\"" << std::endl;
    oss << "node [fontname=\"Monospace\"]" << std::endl;
    oss << "edge [fontname=\"Monospace\"]" << std::endl;
    oss << std::endl;

    int _idx = 0;
    _gvRecurse(oss, this, labelMap, rootNode(), &_idx);

    oss << "}" << std::endl;

    return oss.str();
}
