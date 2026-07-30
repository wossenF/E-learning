#include <vector>
#include <cstdint>

uint32_t mix32(const uint8_t* data, size_t n) {
  uint32_t h = 2166136261u;
  for (size_t i=0;i<n;i++) h = (h ^ data[i]) * 16777619u;
  return h;
}

// filler
// filler
// filler
