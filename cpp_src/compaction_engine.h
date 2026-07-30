#ifndef COMPACTION_ENGINE_H
#define COMPACTION_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

namespace temene {

struct SstableEntry {
  std::string key;
  std::string value;
  uint64_t timestamp;
  bool isTombstone{false};
};

class SstableReader {
public:
  explicit SstableReader(std::vector<SstableEntry> entries);
  bool get(const std::string& key, SstableEntry& outEntry) const;
  const std::vector<SstableEntry>& getEntries() const { return entries_; }

private:
  std::vector<SstableEntry> entries_;
};

class LevelCompactionEngine {
public:
  void addLevel0Sstable(std::shared_ptr<SstableReader> sstable);
  std::shared_ptr<SstableReader> mergeCompaction(const std::vector<std::shared_ptr<SstableReader>>& inputTables);

  size_t getLevel0Count() const { return level0Tables_.size(); }

private:
  std::vector<std::shared_ptr<SstableReader>> level0Tables_;
};

} // namespace temene

#endif // COMPACTION_ENGINE_H
