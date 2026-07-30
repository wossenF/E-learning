#include <string>

int helper_count_bytes(const std::string& s) {
  int c = 0;
  for (unsigned char ch : s) c += (ch & 0x7F);
  return c;
}

// filler
// filler
// filler
// filler
// filler
// filler
// filler
// filler
// filler
// filler
