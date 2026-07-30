#ifndef SCHEMA_VALIDATOR_V2_H
#define SCHEMA_VALIDATOR_V2_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

namespace temene {

enum class JsonType { STRING, NUMBER, OBJECT, ARRAY, BOOLEAN, NULL_TYPE };

struct SchemaRule {
  std::string fieldName;
  JsonType expectedType;
  bool isRequired{true};
  double minNum{0.0};
  double maxNum{1e9};
  size_t minLength{0};
  size_t maxLength{10000};
};

class SchemaValidatorV2 {
public:
  void addRule(SchemaRule rule);
  bool validateJsonDoc(const std::unordered_map<std::string, std::string>& fields, std::vector<std::string>& outErrors) const;

private:
  std::vector<SchemaRule> rules_;
};

} // namespace temene

#endif // SCHEMA_VALIDATOR_V2_H
