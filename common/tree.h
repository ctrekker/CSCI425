#pragma once

#include <vector>
#include <string>
#include <map>
#include <cstdint>


class ParseTree {
    private:
        std::vector<std::vector<int>> adj;
        std::map<int, int> parentMap; // to make parent search quick
        std::vector<int> labels;
    public:
        int rootNode();
        std::vector<int>* getChildren(int nodeId);
        int getParent(int nodeId);
        int addNode(int label);
        void addChild(int parent, int child);
        bool isLeaf(int node);
        std::string toString();
        std::string toString(std::map<int, std::string> labelMap);
        std::string toString(std::map<int, std::string> labelMap, int node, int level);
};
