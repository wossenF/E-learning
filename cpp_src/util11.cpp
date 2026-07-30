#include <string>

int count_alpha(const std::string& s) { int c=0; for (char ch: s) if (std::isalpha((unsigned char)ch)) ++c; return c; }

// filler
// filler
