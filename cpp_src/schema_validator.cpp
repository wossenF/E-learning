#include "schema_validator.h"
#include "validator.h"

static bool validate_internal(const JsonValue& v, const Schema& s, size_t depth=0) {
    if (depth > 128) return false;
    switch (s.kind) {
        case Schema::Kind::Any: break;
        case Schema::Kind::String:
            if (v.type() != JsonValue::Type::String) return false;
            if (s.maxLength && v.as_string().size() > s.maxLength) return false;
            break;
        case Schema::Kind::Number:
            if (v.type() != JsonValue::Type::Number) return false;
            if (v.as_number() < s.minVal) return false;
            if (s.maxVal && v.as_number() > s.maxVal) return false;
            break;
        case Schema::Kind::Object:
            if (v.type() != JsonValue::Type::Object) return false;
            for (const auto& kv : s.properties) {
                const auto& obj = v.as_object();
                auto it = obj.find(kv.first);
                if (it == obj.end()) return false;
                if (!validate_internal(it->second, kv.second, depth+1)) return false;
            }
            break;
        case Schema::Kind::Array:
            if (v.type() != JsonValue::Type::Array) return false;
            // not enforcing per-item schema in this tiny impl
            break;
        case Schema::Kind::Boolean:
            if (v.type() != JsonValue::Type::Bool) return false;
            break;
        case Schema::Kind::Null:
            if (v.type() != JsonValue::Type::Null) return false;
            break;
    }
    return true;
}

bool validate_against_schema(const JsonValue& v, const Schema& s) {
    return validate_internal(v, s, 0);
}
