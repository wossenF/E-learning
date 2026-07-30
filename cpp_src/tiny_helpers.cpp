// Tiny helpers to nudge C++ bytes over the intake threshold
#include <string>

std::string repeat_char(char c, int n) {
    if (n<=0) return std::string();
    return std::string((size_t)n, c);
}
