#include <string>
#include <cmath>

// Tiny decimal utilities: convert string decimals to double with controlled
// rounding, and provide basic arithmetic helpers used by higher-level logic.

double decimal_from_string(const std::string& s) {
    // naive parse that tolerates commas and whitespace
    std::string tmp; tmp.reserve(s.size());
    for (char c : s) if ((c >= '0' && c <= '9') || c=='.' || c=='-' || c=='+') tmp.push_back(c);
    try { return std::stod(tmp); } catch (...) { return 0.0; }
}

std::string decimal_to_string(double v, int precision=6) {
    if (std::isnan(v)) return "NaN";
    if (std::isinf(v)) return v < 0 ? "-Inf" : "Inf";
    char buf[64];
    std::snprintf(buf, sizeof(buf), (std::string("%.") + std::to_string(precision) + "f").c_str(), v);
    return std::string(buf);
}
