#pragma once
#include <cstddef>
#include <cstdint>

// Public API used by the fuzzer.
void process_data(const uint8_t* data, size_t size);
