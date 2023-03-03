#include <iostream>
#include <vector>

#include "serialization.h"

state_set stateIntersect(state_set s1, state_set s2) {
    state_set out;
    for (int s : s1) {
        if (s2.find(s) != s2.end()) {
            out.insert(s);
        }
    }
    return out;
}
