#ifndef DOC_STORE_H
#define DOC_STORE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace temene {

struct JsonDocField {
  std::string path;
  std::string value;
};

class DocumentStoreEngine {
public:
  void insertDocument(const std::string& docId, const std::string& jsonContent);
  std::string getDocument(const std::string& docId) const;

  std::vector<std::string> queryByField(const std::string& path, const std::string& expectedValue) const;
  bool deleteDocument(const std::string& docId);

  size_t documentCount() const { return documents_.size(); }

private:
  std::unordered_map<std::string, std::string> documents_;
  std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> pathIndex_; // path -> { val -> [docIds] }

  std::vector<JsonDocField> extractFields(const std::string& json) const;
};

} // namespace temene

#endif // DOC_STORE_H
