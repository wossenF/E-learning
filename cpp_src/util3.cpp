#include <string>
#include <algorithm>

std::string to_lower_copy(const std::string& s) {
  std::string out = s;
  std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
  return out;
}

// filler
// filler
// filler
// filler
// filler
