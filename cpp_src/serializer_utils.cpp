#include "serializer.h"
#include <sstream>
#include <iomanip>

// small helpers for serializing values for human-readable output

std::string serialize_value_readable(const Value& v) {
    if (std::holds_alternative<std::nullptr_t>(v)) return "null";
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v)?"true":"false";
    if (std::holds_alternative<long long>(v)) return std::to_string(std::get<long long>(v));
    if (std::holds_alternative<double>(v)) { std::ostringstream ss; ss<<std::fixed<<std::setprecision(6)<<std::get<double>(v); return ss.str(); }
    if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v);
    return std::string();
}
