#include "serializer.h"
#include <sstream>

static void append_escaped(std::string& out, const std::string& s) {
    out.push_back('"');
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8]; std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else out.push_back((char)c);
        }
    }
    out.push_back('"');
}

static void serialize_impl(const JsonValue& v, std::string& out) {
    switch (v.type()) {
        case JsonValue::Type::Null: out += "null"; break;
        case JsonValue::Type::Bool: out += (v.as_bool() ? "true" : "false"); break;
        case JsonValue::Type::Number: {
            std::ostringstream ss; ss << v.as_number(); out += ss.str(); break;
        }
        case JsonValue::Type::String: append_escaped(out, v.as_string()); break;
        case JsonValue::Type::Array: {
            out.push_back('[');
            const auto& a = v.as_array();
            for (size_t i=0;i<a.size();++i) { if (i) out.push_back(','); serialize_impl(a[i], out); }
            out.push_back(']');
            break;
        }
        case JsonValue::Type::Object: {
            out.push_back('{');
            const auto& o = v.as_object();
            size_t i=0; for (const auto& kv : o) {
                if (i++) out.push_back(','); append_escaped(out, kv.first); out.push_back(':'); serialize_impl(kv.second, out);
            }
            out.push_back('}');
            break;
        }
    }
}

std::string serialize_json_compact(const JsonValue& v) {
    std::string out; out.reserve(256);
    serialize_impl(v, out);
    return out;
}
