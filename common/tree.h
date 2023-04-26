#pragma once

#include <vector>
#include <string>
#include <map>
#include <cstdint>


struct tree_metadata {
    std::string value;
};

const tree_metadata EMPTY_METADATA;


class ParseTree {
    private:
        int rootId = 0;
        std::vector<std::vector<int>> adj;
        std::map<int, int> parentMap; // to make parent search quick
        std::vector<int> labels;
        std::vector<tree_metadata> metadata;
    public:
        int rootNode();
        void setRoot(int rootId);
        std::vector<int>* getChildren(int nodeId);
        int getParent(int nodeId);
        int getLabel(int node);
        int addNode(int label, tree_metadata meta);
        void addChild(int parent, int child);
        void insertChild(int parent, int child, int pos);
        void removeChild(int parent, int child);
        tree_metadata* getMetadata(int node);
        bool isLeaf(int node);
        std::string toString();
        std::string toString(std::map<int, std::string> labelMap);
        std::string toString(std::map<int, std::string> labelMap, int node, int level);
        std::string toGraphviz(std::map<int, std::string> labelMap);
};
