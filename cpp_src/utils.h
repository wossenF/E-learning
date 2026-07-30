#pragma once
#include <string>
#include <cstdint>

// Utility helpers used by substrate modules.
std::string hexdump(const uint8_t* data, size_t size, size_t max_bytes=64);
bool is_valid_utf8(const uint8_t* data, size_t size);
