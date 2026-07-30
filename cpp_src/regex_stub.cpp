#include <string>
#include <vector>

// Tiny regex-like matcher supporting only '.' and '*' operators for testing.
bool simple_match(const std::string& pattern, const std::string& text) {
    // recursive NFA-like implementation
    std::function<bool(size_t,size_t)> match = [&](size_t pi, size_t ti)->bool{
        if (pi == pattern.size()) return ti == text.size();
        bool first = (ti < text.size()) && (pattern[pi]=='.' || pattern[pi]==text[ti]);
        if (pi+1 < pattern.size() && pattern[pi+1]=='*') {
            // try zero or more
            if (match(pi+2, ti)) return true;
            if (first && match(pi, ti+1)) return true;
            return false;
        } else {
            return first && match(pi+1, ti+1);
        }
    };
    return match(0,0);
}
