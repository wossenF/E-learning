#include "json_parser.h"
#include <cctype>
#include <sstream>

namespace temene {

std::string JsonValue::serialize() const {
  if (type == JsonValueType::NULL_VAL) return "null";
  if (type == JsonValueType::BOOLEAN) return boolVal ? "true" : "false";
  if (type == JsonValueType::NUMBER) {
    std::ostringstream ss;
    ss << numVal;
    return ss.str();
  }
  if (type == JsonValueType::STRING) {
    return "\"" + strVal + "\""; // simplistic escaping
  }
  if (type == JsonValueType::ARRAY) {
    std::string res = "[";
    for (size_t i = 0; i < arrayVal.size(); ++i) {
      res += arrayVal[i]->serialize();
      if (i + 1 < arrayVal.size()) res += ",";
    }
    res += "]";
    return res;
  }
  if (type == JsonValueType::OBJECT) {
    std::string res = "{";
    bool first = true;
    for (const auto& [k, v] : objectVal) {
      if (!first) res += ",";
      res += "\"" + k + "\":" + v->serialize();
      first = false;
    }
    res += "}";
    return res;
  }
  return "";
}

void JsonParserEngine::skipWhitespace() {
  while (pos_ < data_.size() && std::isspace(data_[pos_])) {
    pos_++;
  }
}

std::shared_ptr<JsonValue> JsonParserEngine::parse(const std::string& jsonString) {
  data_ = jsonString;
  pos_ = 0;
  return parseValue();
}

std::shared_ptr<JsonValue> JsonParserEngine::parseValue() {
  skipWhitespace();
  if (pos_ >= data_.size()) return nullptr;

  char c = data_[pos_];
  if (c == '{') return parseObject();
  if (c == '[') return parseArray();
  if (c == '"') {
    auto val = std::make_shared<JsonValue>();
    val->type = JsonValueType::STRING;
    val->strVal = parseString();
    return val;
  }
  if (c == 't' || c == 'f') {
    auto val = std::make_shared<JsonValue>();
    val->type = JsonValueType::BOOLEAN;
    if (data_.compare(pos_, 4, "true") == 0) {
      val->boolVal = true;
      pos_ += 4;
    } else if (data_.compare(pos_, 5, "false") == 0) {
      val->boolVal = false;
      pos_ += 5;
    }
    return val;
  }
  if (c == 'n') {
    auto val = std::make_shared<JsonValue>();
    val->type = JsonValueType::NULL_VAL;
    if (data_.compare(pos_, 4, "null") == 0) {
      pos_ += 4;
    }
    return val;
  }
  
  if (c == '-' || std::isdigit(c)) {
    auto val = std::make_shared<JsonValue>();
    val->type = JsonValueType::NUMBER;
    val->numVal = parseNumber();
    return val;
  }
  
  return nullptr;
}

std::string JsonParserEngine::parseString() {
  pos_++; // skip quote
  std::string result;
  while (pos_ < data_.size() && data_[pos_] != '"') {
    result += data_[pos_++];
  }
  if (pos_ < data_.size() && data_[pos_] == '"') pos_++; // skip quote
  return result;
}

double JsonParserEngine::parseNumber() {
  size_t start = pos_;
  while (pos_ < data_.size() && (std::isdigit(data_[pos_]) || data_[pos_] == '-' || data_[pos_] == '.' || data_[pos_] == 'e' || data_[pos_] == 'E' || data_[pos_] == '+')) {
    pos_++;
  }
  std::string numStr = data_.substr(start, pos_ - start);
  return std::stod(numStr);
}

std::shared_ptr<JsonValue> JsonParserEngine::parseObject() {
  auto obj = std::make_shared<JsonValue>();
  obj->type = JsonValueType::OBJECT;
  
  pos_++; // skip '{'
  skipWhitespace();
  
  if (pos_ < data_.size() && data_[pos_] == '}') {
    pos_++;
    return obj;
  }
  
  while (pos_ < data_.size()) {
    skipWhitespace();
    if (data_[pos_] != '"') throw std::runtime_error("Expected string key in object");
    
    std::string key = parseString();
    skipWhitespace();
    
    if (pos_ >= data_.size() || data_[pos_] != ':') throw std::runtime_error("Expected ':' in object");
    pos_++; // skip ':'
    
    auto val = parseValue();
    if (val) obj->objectVal[key] = val;
    
    skipWhitespace();
    if (pos_ >= data_.size() || data_[pos_] == '}') {
      if (pos_ < data_.size()) pos_++; // skip '}'
      break;
    }
    if (data_[pos_] != ',') throw std::runtime_error("Expected ',' or '}' in object");
    pos_++; // skip ','
  }
  
  return obj;
}

std::shared_ptr<JsonValue> JsonParserEngine::parseArray() {
  auto arr = std::make_shared<JsonValue>();
  arr->type = JsonValueType::ARRAY;
  
  pos_++; // skip '['
  skipWhitespace();
  
  if (pos_ < data_.size() && data_[pos_] == ']') {
    pos_++;
    return arr;
  }
  
  while (pos_ < data_.size()) {
    auto val = parseValue();
    if (val) arr->arrayVal.push_back(val);
    
    skipWhitespace();
    if (pos_ >= data_.size() || data_[pos_] == ']') {
      if (pos_ < data_.size()) pos_++; // skip ']'
      break;
    }
    if (data_[pos_] != ',') throw std::runtime_error("Expected ',' or ']' in array");
    pos_++; // skip ','
  }
  
  return arr;
}

} // namespace temene
