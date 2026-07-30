#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

// UTF-8 helper utilities useful for parser fuzzing.
size_t utf8_char_length(uint8_t lead);
bool is_valid_utf8_sequence(const uint8_t* data, size_t size);
std::string utf8_sanitize(const uint8_t* data, size_t size);
