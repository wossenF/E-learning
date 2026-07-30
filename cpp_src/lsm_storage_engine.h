#ifndef LSM_STORAGE_ENGINE_H
#define LSM_STORAGE_ENGINE_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>

namespace temene {

class LsmMemTable {
public:
  void put(const std::string& key, const std::string& value);
  bool get(const std::string& key, std::string& outValue) const;
  void remove(const std::string& key);

  size_t byteSize() const { return currentByteSize_; }
  void clear();

  const std::map<std::string, std::string>& getEntries() const { return memMap_; }

private:
  std::map<std::string, std::string> memMap_;
  size_t currentByteSize_{0};
};

class LsmStorageEngine {
public:
  explicit LsmStorageEngine(size_t memtableThresholdBytes = 64 * 1024);

  void put(const std::string& key, const std::string& value);
  std::string get(const std::string& key);

  void flushMemTable();

private:
  size_t thresholdBytes_;
  LsmMemTable activeMemTable_;
  std::vector<std::map<std::string, std::string>> flushedSstables_;
};

} // namespace temene

#endif // LSM_STORAGE_ENGINE_H
