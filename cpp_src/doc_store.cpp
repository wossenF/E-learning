#include "doc_store.h"
#include <algorithm>
#include <sstream>

namespace temene {

std::vector<JsonDocField> DocumentStoreEngine::extractFields(const std::string& json) const {
  std::vector<JsonDocField> fields;
  size_t idx = 0;
  while (idx < json.size()) {
    size_t keyStart = json.find('"', idx);
    if (keyStart == std::string::npos) break;
    size_t keyEnd = json.find('"', keyStart + 1);
    if (keyEnd == std::string::npos) break;

    std::string key = json.substr(keyStart + 1, keyEnd - keyStart - 1);
    size_t colon = json.find(':', keyEnd);
    if (colon == std::string::npos) break;

    size_t valStart = json.find_first_not_of(" \t\r\n", colon + 1);
    if (valStart == std::string::npos) break;

    std::string val;
    if (json[valStart] == '"') {
      size_t valEnd = json.find('"', valStart + 1);
      if (valEnd != std::string::npos) {
        val = json.substr(valStart + 1, valEnd - valStart - 1);
        idx = valEnd + 1;
      }
    } else {
      size_t valEnd = json.find_first_of(",}\r\n", valStart);
      if (valEnd != std::string::npos) {
        val = json.substr(valStart, valEnd - valStart);
        idx = valEnd + 1;
      } else {
        val = json.substr(valStart);
        idx = json.size();
      }
    }

    fields.push_back({key, val});
  }

  return fields;
}

void DocumentStoreEngine::insertDocument(const std::string& docId, const std::string& jsonContent) {
  documents_[docId] = jsonContent;
  auto fields = extractFields(jsonContent);

  for (const auto& f : fields) {
    pathIndex_[f.path][f.value].push_back(docId);
  }
}

std::string DocumentStoreEngine::getDocument(const std::string& docId) const {
  auto it = documents_.find(docId);
  if (it != documents_.end()) return it->second;
  return "";
}

std::vector<std::string> DocumentStoreEngine::queryByField(const std::string& path, const std::string& expectedValue) const {
  auto pathIt = pathIndex_.find(path);
  if (pathIt == pathIndex_.end()) return {};

  auto valIt = pathIt->second.find(expectedValue);
  if (valIt == pathIt->second.end()) return {};

  return valIt->second;
}

bool DocumentStoreEngine::deleteDocument(const std::string& docId) {
  return documents_.erase(docId) > 0;
}

} // namespace temene
