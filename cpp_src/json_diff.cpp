#include "json_value.h"
#include <vector>
#include <string>
#include <map>

// Lightweight JSON diff: compares two JsonValue trees and emits a list of
// textual operations describing differences. This is intentionally simple but
// exercises traversal and allocation.

struct DiffOp {
    enum class Op { Add, Remove, Replace } op;
    std::string path;
    JsonValue value;
};

static void diff_internal(const JsonValue& a, const JsonValue& b, const std::string& path, std::vector<DiffOp>& out) {
    if (a.type() != b.type()) {
        out.push_back({DiffOp::Op::Replace, path, b});
        return;
    }
    switch (a.type()) {
        case JsonValue::Type::Object: {
            const auto& oa = a.as_object();
            const auto& ob = b.as_object();
            for (const auto& kv : oa) {
                auto it = ob.find(kv.first);
                std::string child = path.empty() ? kv.first : path + "." + kv.first;
                if (it == ob.end()) out.push_back({DiffOp::Op::Remove, child, JsonValue()});
                else diff_internal(kv.second, it->second, child, out);
            }
            for (const auto& kv : ob) if (oa.find(kv.first) == oa.end()) {
                std::string child = path.empty() ? kv.first : path + "." + kv.first;
                out.push_back({DiffOp::Op::Add, child, kv.second});
            }
            break;
        }
        case JsonValue::Type::Array: {
            const auto& aa = a.as_array();
            const auto& ab = b.as_array();
            size_t n = std::min(aa.size(), ab.size());
            for (size_t i=0;i<n;++i) {
                diff_internal(aa[i], ab[i], path + "[" + std::to_string(i) + "]", out);
            }
            for (size_t i=n;i<aa.size();++i) out.push_back({DiffOp::Op::Remove, path + "[" + std::to_string(i) + "]", JsonValue()});
            for (size_t i=n;i<ab.size();++i) out.push_back({DiffOp::Op::Add, path + "[" + std::to_string(i) + "]", ab[i]});
            break;
        }
        case JsonValue::Type::String:
            if (a.as_string() != b.as_string()) out.push_back({DiffOp::Op::Replace, path, b});
            break;
        case JsonValue::Type::Number:
            if (a.as_number() != b.as_number()) out.push_back({DiffOp::Op::Replace, path, b});
            break;
        case JsonValue::Type::Bool:
            if (a.as_bool() != b.as_bool()) out.push_back({DiffOp::Op::Replace, path, b});
            break;
        case JsonValue::Type::Null:
            break;
    }
}

std::vector<DiffOp> compute_diff(const JsonValue& a, const JsonValue& b) {
    std::vector<DiffOp> out;
    diff_internal(a, b, "", out);
    return out;
}
