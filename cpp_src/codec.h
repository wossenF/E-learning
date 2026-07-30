#pragma once
#include <string>
#include <vector>

// Small codec utilities for fuzzing: base64 (encode/decode) and simple hex.
std::string base64_encode(const std::vector<uint8_t>& data);
std::vector<uint8_t> base64_decode(const std::string& s);
std::string to_hex(const std::vector<uint8_t>& data);
