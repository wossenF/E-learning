#include "utils.h"
#include "json_parser.h"
#include "streaming_parser.h"
#include "codec.h"
#include <vector>
#include <random>
#include <chrono>

// Helpers used by fuzzing harnesses to generate interesting mutations and
// to run combined code paths in the substrate.

std::vector<uint8_t> mutate_flip_bits(const uint8_t* data, size_t size, size_t flips) {
    std::vector<uint8_t> out(data, data+size);
    std::mt19937 rng((unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<size_t> idx(0, size?size-1:0);
    std::uniform_int_distribution<int> bit(0,7);
    for (size_t i=0;i<flips && size>0;++i) {
        size_t p = idx(rng);
        int b = bit(rng);
        out[p] ^= (1u << b);
    }
    return out;
}

void exercise_codepaths(const uint8_t* data, size_t size) {
    // Try full-parse
    parse_json_like(data, size);
    // Try streaming parse with small chunk sizes to exercise incremental logic
    StreamingParser sp;
    sp.set_on_start_object([](){});
    sp.set_on_end_object([](){});
    sp.set_on_string([](const std::string& s){});
    for (size_t chunk=1; chunk <= 16; chunk*=2) {
        for (size_t off=0; off < size; off += chunk) sp.feed(data+off, std::min(chunk, size-off));
        sp.finish();
    }
    // Validate utf8 and hexdump
    hexdump(data, size, 32);
    is_valid_utf8(data, size);
}
