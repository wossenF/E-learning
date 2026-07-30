// libFuzzer harness that forwards input into the project's C++ substrate.
#include <cstddef>
#include <cstdint>
#include "../cpp_src/api.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (!data || size == 0) return 0;
    // Forward input bytes into the project's processing function.
    process_data(data, size);
    return 0;
}
