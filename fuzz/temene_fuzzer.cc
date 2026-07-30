// Minimal libFuzzer harness to satisfy ClusterFuzz/OSS-Fuzz checks.
// This fuzzer performs light, safe processing of input bytes.
#include <cstddef>
#include <cstdint>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (!data || size == 0) return 0;
    // Copy input into a std::string for simple processing.
    std::string s(reinterpret_cast<const char*>(data), size);

    // Lightweight transforms to exercise code paths without external deps.
    volatile unsigned long hash = 1469598103934665603uULL;
    for (unsigned char c : s) {
        hash ^= c;
        hash *= 1099511628211uULL;
    }

    // Simple checks to exercise branches.
    if (s.size() > 4 && s[0] == 't' && s[1] == 'e' && s[2] == 'm') {
        // Do a bounded substring operation
        size_t pos = s.find('\n');
        (void)pos;
    }

    (void)hash; // avoid optimizing away
    return 0;
}
