// Small utility functions to bump C++ substrate size
#include <string>
#include <vector>
#include <cctype>

std::string to_lower_copy(const std::string& s) {
    std::string out; out.reserve(s.size());
    for (unsigned char c: s) out.push_back((char)std::tolower(c));
    return out;
}

std::vector<std::string> split_words(const std::string& s) {
    std::vector<std::string> parts; std::string cur;
    for (char ch: s) {
        if (std::isspace((unsigned char)ch)) { if (!cur.empty()) { parts.push_back(cur); cur.clear(); } }
        else cur.push_back(ch);
    }
    if (!cur.empty()) parts.push_back(cur);
    return parts;
}

// simple checksum for quick data sanity checks
unsigned int simple_checksum(const std::vector<unsigned char>& data) {
    unsigned int h=2166136261u;
    for (unsigned char b: data) h = (h ^ b) * 16777619u;
    return h;
}
