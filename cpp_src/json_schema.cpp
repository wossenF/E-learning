// Simple JSON Schema-like validator (subset) to provide realistic C++ substrate.
// Supports types: object, array, string, number, boolean, null
// and simple constraints: required, min/max for numbers, minLength/maxLength for strings.

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <optional>
#include <sstream>
#include <algorithm>
#include <cctype>

using Json = std::variant<std::nullptr_t, bool, double, std::string, std::vector<Json>, std::map<std::string,Json>>;

struct Schema {
    std::string type; // "object","array","string","number","boolean","null"
    std::vector<std::string> required;
    std::map<std::string, Schema> properties; // for object
    std::optional<double> minimum;
    std::optional<double> maximum;
    std::optional<size_t> minLength;
    std::optional<size_t> maxLength;
    std::unique_ptr<Schema> items; // for arrays
};

struct ValidationError {
    std::string path;
    std::string message;
};

static std::string join_path(const std::string& base, const std::string& key) {
    if (base.empty()) return key;
    if (key.empty()) return base;
    std::ostringstream ss; ss << base << "." << key; return ss.str();
}

static bool is_number(const Json& v) { return std::holds_alternative<double>(v); }
static bool is_string(const Json& v) { return std::holds_alternative<std::string>(v); }
static bool is_bool(const Json& v) { return std::holds_alternative<bool>(v); }
static bool is_null(const Json& v) { return std::holds_alternative<std::nullptr_t>(v); }
static bool is_array(const Json& v) { return std::holds_alternative<std::vector<Json>>(v); }
static bool is_object(const Json& v) { return std::holds_alternative<std::map<std::string,Json>>(v); }

void validate_impl(const Json& value, const Schema& schema, const std::string& path, std::vector<ValidationError>& out) {
    // Type check
    if (!schema.type.empty()) {
        bool ok=false;
        if (schema.type=="number") ok = is_number(value);
        else if (schema.type=="string") ok = is_string(value);
        else if (schema.type=="boolean") ok = is_bool(value);
        else if (schema.type=="null") ok = is_null(value);
        else if (schema.type=="array") ok = is_array(value);
        else if (schema.type=="object") ok = is_object(value);
        if (!ok) { out.push_back({path, "type mismatch (expected "+schema.type+")"}); return; }
    }

    if (is_number(value)) {
        double n = std::get<double>(value);
        if (schema.minimum && n < *schema.minimum) out.push_back({path, "number below minimum"});
        if (schema.maximum && n > *schema.maximum) out.push_back({path, "number above maximum"});
    } else if (is_string(value)) {
        const std::string& s = std::get<std::string>(value);
        if (schema.minLength && s.size() < *schema.minLength) out.push_back({path, "string too short"});
        if (schema.maxLength && s.size() > *schema.maxLength) out.push_back({path, "string too long"});
    } else if (is_array(value)) {
        const auto& arr = std::get<std::vector<Json>>(value);
        if (schema.items) {
            for (size_t i=0;i<arr.size();++i) {
                std::ostringstream p; p<<path<<"["<<i<<"]";
                validate_impl(arr[i], *schema.items, p.str(), out);
            }
        }
    } else if (is_object(value)) {
        const auto& obj = std::get<std::map<std::string,Json>>(value);
        // check required
        for (const auto& r : schema.required) {
            if (obj.find(r) == obj.end()) out.push_back({join_path(path,r), "required property missing"});
        }
        for (const auto& prop : schema.properties) {
            auto it = obj.find(prop.first);
            if (it != obj.end()) {
                validate_impl(it->second, prop.second, join_path(path, prop.first), out);
            }
        }
    }
}

std::vector<ValidationError> validate_json(const Json& value, const Schema& schema) {
    std::vector<ValidationError> out;
    validate_impl(value, schema, "", out);
    return out;
}

// Small helper: build a sample schema and validate a simple JSON (used by tests/examples)
Schema build_sample_schema() {
    Schema root; root.type = "object"; root.required = {"id","name"};
    Schema id; id.type = "number"; id.minimum = 0; id.maximum = 1000000; root.properties["id"] = id;
    Schema name; name.type = "string"; name.minLength = 1; name.maxLength = 128; root.properties["name"] = name;
    Schema tags; tags.type = "array"; tags.items = std::make_unique<Schema>(); tags.items->type = "string"; root.properties["tags"] = tags;
    return root;
}

// Minimal JSON builder helpers for examples
Json make_string(const std::string& s) { return Json(s); }
Json make_number(double n) { return Json(n); }
Json make_bool(bool b) { return Json(b); }
Json make_null() { return Json(nullptr); }
Json make_array(const std::vector<Json>& v) { return Json(v); }
Json make_object(const std::map<std::string,Json>& m) { return Json(m); }
