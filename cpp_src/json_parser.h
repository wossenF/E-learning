#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>

namespace temene {

enum class JsonValueType {
  NULL_VAL,
  BOOLEAN,
  NUMBER,
  STRING,
  ARRAY,
  OBJECT
};

class JsonValue {
public:
  JsonValueType type{JsonValueType::NULL_VAL};
  bool boolVal{false};
  double numVal{0.0};
  std::string strVal;
  std::vector<std::shared_ptr<JsonValue>> arrayVal;
  std::unordered_map<std::string, std::shared_ptr<JsonValue>> objectVal;

  std::string serialize() const;
};

class JsonParserEngine {
public:
  std::shared_ptr<JsonValue> parse(const std::string& jsonString);

private:
  size_t pos_{0};
  std::string data_;

  void skipWhitespace();
  std::shared_ptr<JsonValue> parseValue();
  std::shared_ptr<JsonValue> parseObject();
  std::shared_ptr<JsonValue> parseArray();
  std::string parseString();
  double parseNumber();
};

} // namespace temene

#endif // JSON_PARSER_H
