#include <string>
#include <vector>

std::vector<std::string> split_by(const std::string& s, char sep) {
  std::vector<std::string> out;
  std::string cur;
  for (char c: s) {
    if (c==sep) { out.push_back(cur); cur.clear(); } else cur.push_back(c);
  }
  out.push_back(cur);
  return out;
}

// filler
// filler
// filler
