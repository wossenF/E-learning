#pragma once
#include <string>
#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>
#include <map>

// A tiny, permissive JSON-like parser for fuzzing exercises.
// Not fully compliant — intentionally simple to provide interesting parsing logic.

using JsonValue = std::variant<std::nullptr_t, bool, double, std::string, std::vector<std::variant<std::nullptr_t, bool, double, std::string>>, std::map<std::string, std::string>>;

// Parse input bytes; returns true on success, false on parse error.
// The parsed result is not used, but the parser exercises string handling,
// numeric parsing, and nested structures — useful for fuzzing memory issues.
bool parse_json_like(const uint8_t* data, size_t size);
