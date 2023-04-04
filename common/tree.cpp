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

// TODO: make this efficient
int ParseTree::getParent(int nodeId) {
    return parentMap[nodeId];
}

int ParseTree::addNode(int label) {
    std::vector<int> empty;
    adj.push_back(empty);
    labels.push_back(label);
    return adj.size() - 1;
}

void ParseTree::addChild(int parent, int child) {
    adj[parent].push_back(child);
    parentMap[child] = parent;
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
    // std::vector<int> idxStack = {0};
    // std::vector<int> nodeStack = {rootNode()};
    // ss << "(";
    // while (idxStack.size() > 0) {
    //     int currentNode = nodeStack[nodeStack.size() - 1];
    //     int currentChild = idxStack[idxStack.size() - 1];
    //     std::cout << idxStack << "," << nodeStack << std::endl;
    //     if (labelMap.find(currentChild) != labelMap.end()) {
    //         ss << labelMap[currentChild];
    //     }
    //     else {
    //         std::cout << adj << labels << std::endl;
    //         ss << labels[adj[currentNode][currentChild]];
    //     }
    //     std::cout << isLeaf(adj[currentNode][currentChild]);
    //     if (currentChild < adj[currentNode].size() - 1) {
    //         idxStack[idxStack.size() - 1]++;
    //     }
    //     else {
    //         ss << ")";
    //         nodeStack.pop_back();
    //         idxStack.pop_back();
    //     }
    //     std::cout << "PREL" << std::endl;
    //     if (!isLeaf(adj[currentNode][currentChild])) {
    //         nodeStack.push_back(currentChild);
    //         idxStack.push_back(0);
    //         ss << "(";
    //     }
    // }
        

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
