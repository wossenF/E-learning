#pragma once
#include "json_value.h"
#include <string>

// A tiny schema language: supports 'type' and 'maxLength' for strings,
// 'min'/'max' for numbers, and nested object/array constraints.
class Schema {
public:
    enum class Kind { Any, String, Number, Boolean, Null, Object, Array };
    Schema();
    Kind kind = Kind::Any;
    size_t maxLength = 0; // 0 means unset
    double minVal = 0.0;
    double maxVal = 0.0;
    // For objects/arrays, simple map from keys to schemas.
    std::map<std::string, Schema> properties;
};

bool validate_against_schema(const JsonValue& v, const Schema& s);
