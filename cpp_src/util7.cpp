#include <string>

int count_digits(const std::string& s) {
  int c=0; for (char ch: s) if (ch>='0' && ch<='9') ++c; return c;
}

// filler
// filler
// filler
