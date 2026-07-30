#include <string>
#include <sstream>

std::string join_with(const std::vector<std::string>& parts, char sep) {
  std::ostringstream os;
  bool first = true;
  for (auto &p: parts) {
    if (!first) os << sep;
    os << p;
    first = false;
  }
  return os.str();
}

// filler
// filler
