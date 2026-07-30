#include "inmem_db.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

namespace inmembase {

// Library of scalar functions usable in queries. For now these are util
// functions implemented in C++ to make query testing and fuzzing richer.

long long func_abs_ll(long long x) { return x<0?-x:x; }
double func_pow(double base, double exp) { return std::pow(base, exp); }
std::string func_concat(const std::vector<std::string>& parts, const std::string& sep) {
    std::string out; for (size_t i=0;i<parts.size();++i) { if (i) out += sep; out += parts[i]; } return out;
}

bool func_like(const std::string& value, const std::string& pattern) {
    // very simple pattern: support '%' wildcard only
    size_t ppos = pattern.find('%');
    if (ppos == std::string::npos) return value == pattern;
    std::string before = pattern.substr(0, ppos); std::string after = pattern.substr(ppos+1);
    if (!before.empty() && value.rfind(before,0)!=0) return false;
    if (!after.empty() && value.size() < after.size()) return false;
    if (!after.empty()) return value.substr(value.size()-after.size()) == after;
    return true;
}

} // namespace inmembase
