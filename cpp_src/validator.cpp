#include "validator.h"
#include <stack>

static bool validate_impl(const JsonValue& v, size_t depth, size_t max_depth) {
    if (depth > max_depth) return false;
    switch (v.type()) {
        case JsonValue::Type::Null: return true;
        case JsonValue::Type::Bool: return true;
        case JsonValue::Type::Number: return true;
        case JsonValue::Type::String: return v.as_string().size() < 1<<20; // cap strings
        case JsonValue::Type::Array: {
            const auto& a = v.as_array();
            if (a.size() > 1<<20) return false;
            for (const auto& e : a) if (!validate_impl(e, depth+1, max_depth)) return false;
            return true;
        }
        case JsonValue::Type::Object: {
            const auto& o = v.as_object();
            if (o.size() > 1<<20) return false;
            for (const auto& kv : o) if (!validate_impl(kv.second, depth+1, max_depth)) return false;
            return true;
        }
    }
    return false;
}

bool validate_json(const JsonValue& v, size_t max_depth) {
    return validate_impl(v, 0, max_depth);
}
