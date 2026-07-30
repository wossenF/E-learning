#include "path_query.h"
#include <sstream>

static std::vector<std::string> split_dotted(const std::string& s) {
    std::vector<std::string> out; std::string cur;
    std::istringstream ss(s);
    while (std::getline(ss, cur, '.')) out.push_back(cur);
    return out;
}

std::vector<JsonValue> query_path(const JsonValue& root, const std::string& path) {
    std::vector<JsonValue> results;
    const JsonValue* current = &root;
    auto parts = split_dotted(path);
    for (size_t i=0;i<parts.size();++i) {
        const std::string& p = parts[i];
        if (current->type() == JsonValue::Type::Object) {
            const auto& obj = current->as_object();
            auto it = obj.find(p);
            if (it == obj.end()) return {};
            current = &it->second;
        } else if (current->type() == JsonValue::Type::Array) {
            // interpret p as index
            try {
                size_t idx = std::stoul(p);
                const auto& arr = current->as_array();
                if (idx >= arr.size()) return {};
                current = &arr[idx];
            } catch (...) { return {}; }
        } else return {};
    }
    results.push_back(*current);
    return results;
}
