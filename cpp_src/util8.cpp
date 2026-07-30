#include <cstddef>
#include <cstdint>

size_t sum_bytes(const uint8_t* d, size_t n) { size_t s=0; for(size_t i=0;i<n;++i) s+=d[i]; return s; }

// filler
// filler
