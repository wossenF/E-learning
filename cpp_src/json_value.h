#pragma once
#include <string>
#include <variant>
#include <vector>
#include <map>

class JsonValue;

using JsonArray = std::vector<JsonValue>;
using JsonObject = std::map<std::string, JsonValue>;

class JsonValue {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };
    JsonValue();
    JsonValue(std::nullptr_t);
    JsonValue(bool b);
    JsonValue(double n);
    JsonValue(const std::string& s);
    JsonValue(std::string&& s);
    JsonValue(const JsonArray& a);
    JsonValue(JsonArray&& a);
    JsonValue(const JsonObject& o);
    JsonValue(JsonObject&& o);

    Type type() const;
    bool as_bool() const;
    double as_number() const;
    const std::string& as_string() const;
    const JsonArray& as_array() const;
    const JsonObject& as_object() const;

private:
    Type type_;
    std::variant<std::nullptr_t, bool, double, std::string, JsonArray, JsonObject> value_;
};
