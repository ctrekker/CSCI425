#pragma once

#include <set>

template <typename T>
void unionMutating(std::set<T> &set1, std::set<T> &set2) {
    for (T elem : set2) {
        set1.insert(elem);
    }
}
