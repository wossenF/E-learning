// libFuzzer harness that forwards input into the project's C++ substrate.
#include <cstddef>
#include <cstdint>
#include "../cpp_src/json_parser.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (!data || size == 0) return 0;
    // Feed the fuzzer input to the JSON-like parser.
    parse_json_like(data, size);
    return 0;
}
