#include "json_value.h"
#include <utility>

JsonValue::JsonValue() : type_(Type::Null), value_(nullptr) {}
JsonValue::JsonValue(std::nullptr_t) : type_(Type::Null), value_(nullptr) {}
JsonValue::JsonValue(bool b) : type_(Type::Bool), value_(b) {}
JsonValue::JsonValue(double n) : type_(Type::Number), value_(n) {}
JsonValue::JsonValue(const std::string& s) : type_(Type::String), value_(s) {}
JsonValue::JsonValue(std::string&& s) : type_(Type::String), value_(std::move(s)) {}
JsonValue::JsonValue(const JsonArray& a) : type_(Type::Array), value_(a) {}
JsonValue::JsonValue(JsonArray&& a) : type_(Type::Array), value_(std::move(a)) {}
JsonValue::JsonValue(const JsonObject& o) : type_(Type::Object), value_(o) {}
JsonValue::JsonValue(JsonObject&& o) : type_(Type::Object), value_(std::move(o)) {}

JsonValue::Type JsonValue::type() const { return type_; }
bool JsonValue::as_bool() const { return std::get<bool>(value_); }
double JsonValue::as_number() const { return std::get<double>(value_); }
const std::string& JsonValue::as_string() const { return std::get<std::string>(value_); }
const JsonArray& JsonValue::as_array() const { return std::get<JsonArray>(value_); }
const JsonObject& JsonValue::as_object() const { return std::get<JsonObject>(value_); }
