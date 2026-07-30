#include <string>
#include <algorithm>
#include <cctype>

std::string to_lower_copy(const std::string& s) {
    std::string out = s; std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); }); return out;
}

std::string trim_copy(const std::string& s) {
    size_t b = 0; while (b < s.size() && std::isspace((unsigned char)s[b])) ++b;
    size_t e = s.size(); while (e > b && std::isspace((unsigned char)s[e-1])) --e;
    return s.substr(b, e-b);
}

bool starts_with(const std::string& s, const std::string& prefix) {
    if (s.size() < prefix.size()) return false; return std::equal(prefix.begin(), prefix.end(), s.begin());
}
