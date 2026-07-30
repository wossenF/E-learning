#pragma once
#include <string>
#include "json_value.h"

// Serialize JsonValue into a compact JSON string.
std::string serialize_json_compact(const JsonValue& v);
