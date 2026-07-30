#pragma once
#include <vector>
#include <algorithm>

template<typename T>
void stable_remove_duplicates(std::vector<T>& v) {
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
}
