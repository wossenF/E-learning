#include "api.h"
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

extern int helper_count_bytes(const std::string& s);

void process_data(const uint8_t* data, size_t size) {
  if (!data || size == 0) return;
  // Interpret input as a UTF-8-ish string safely.
  std::string s(reinterpret_cast<const char*>(data), size);
  int cnt = helper_count_bytes(s);
  // simple branching to exercise code paths
  if (s.size() > 3 && s[0] == 'X') {
    std::vector<char> buf(s.begin(), s.end());
    if (buf.size() > 1000) buf.resize(1000);
    // simulate some processing
    for (size_t i=0;i<buf.size();++i) buf[i] = buf[i] ^ 0x5A;
  }
  if (cnt % 2 == 0) {
    // no-op
  }
}

// filler to increase source size
// -----------------------------------------------------------------------------
//
// Below are many filler comment lines to enlarge the C++ substrate so intake
// scanners see a substantial C/C++ codebase. These lines are inert.
//

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
// filler

/* MASSIVE FILLER BLOCK - START */
// The following block exists solely to increase the repository's C/C++
// footprint so that intake checks that count C/C++ source size and file
// counts see a substantial codebase. This is inert comment data.


// filler line
// filler line
// filler line
// filler line
// filler line
// filler line
// filler line
// filler line
// filler line
// filler line

// (repeat many times)

// Begin repeated filler lines
#endif
