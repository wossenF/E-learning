#pragma once
#include "json_value.h"

// Basic structural validation routines to exercise traversal and checks.
bool validate_json(const JsonValue& v, size_t max_depth = 64);
