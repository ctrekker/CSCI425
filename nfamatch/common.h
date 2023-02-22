#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <unordered_set>

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    os << "[";
    for (int i = 0; i < v.size(); ++i) {
        os << v[i];
        if (i != v.size() - 1)
            os << ", ";
    }
    os << "]";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<T>& v) {
    std::vector<T> vec;
    auto itr = v.begin();
    for (;itr != v.end(); itr++) {
        vec.push_back(*itr);
    }

    os << vec;
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& v) {
    std::vector<T> vec;
    auto itr = v.begin();
    for (;itr != v.end(); itr++) {
        vec.push_back(*itr);
    }

    os << vec;
    return os;
}

struct Transition {
    int to;
    char token;
};

struct StateInfo {
    bool accepting;
    bool start;
};


typedef std::set<int> state_set;

state_set stateIntersect(state_set s1, state_set s2);

