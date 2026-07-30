#include "schema_validator_v2.h"

namespace temene {

void SchemaValidatorV2::addRule(SchemaRule rule) {
  rules_.push_back(rule);
}

bool SchemaValidatorV2::validateJsonDoc(
  const std::unordered_map<std::string, std::string>& fields,
  std::vector<std::string>& outErrors
) const {
  bool isValid = true;

  for (const auto& rule : rules_) {
    auto it = fields.find(rule.fieldName);
    if (it == fields.end()) {
      if (rule.isRequired) {
        outErrors.push_back("Missing required field: " + rule.fieldName);
        isValid = false;
      }
      continue;
    }

    const std::string& val = it->second;

    if (rule.expectedType == JsonType::STRING) {
      if (val.size() < rule.minLength || val.size() > rule.maxLength) {
        outErrors.push_back("Field " + rule.fieldName + " length out of bounds.");
        isValid = false;
      }
    } else if (rule.expectedType == JsonType::NUMBER) {
      try {
        double d = std::stod(val);
        if (d < rule.minNum || d > rule.maxNum) {
          outErrors.push_back("Field " + rule.fieldName + " numeric value out of bounds.");
          isValid = false;
        }
      } catch (...) {
        outErrors.push_back("Field " + rule.fieldName + " is not a valid number.");
        isValid = false;
      }
    }
  }

  return isValid;
}

} // namespace temene
