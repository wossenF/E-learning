#include "json_value.h"
#include <utility>

// Merge b into a respecting object properties (shallow merge for arrays).
JsonValue merge_json(JsonValue a, const JsonValue& b) {
    if (a.type() != JsonValue::Type::Object || b.type() != JsonValue::Type::Object) return b;
    auto result = a.as_object();
    const auto& other = b.as_object();
    for (const auto& kv : other) {
        auto it = result.find(kv.first);
        if (it == result.end()) result[kv.first] = kv.second;
        else {
            // If both are objects, merge recursively
            if (it->second.type() == JsonValue::Type::Object && kv.second.type() == JsonValue::Type::Object) {
                it->second = merge_json(it->second, kv.second);
            } else {
                it->second = kv.second;
            }
        }
    }
    return JsonValue(std::move(result));
}
