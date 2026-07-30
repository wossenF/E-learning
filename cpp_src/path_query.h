#pragma once
#include "json_value.h"
#include <string>
#include <vector>

// Simple JSONPath-like query: dot-separated keys and numeric indices.
// Examples: "root.items.0.name" or "users.3.email"
std::vector<JsonValue> query_path(const JsonValue& root, const std::string& path);
